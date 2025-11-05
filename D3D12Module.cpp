#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"


D3D12Module::D3D12Module(HWND hwnd): hWnd (hwnd), currentExecution(0), currentFrameBuffIndex(0)
{
}

D3D12Module::~D3D12Module() // REMEMBER THAT WE HAVE A CLEANUP() FUNCTION!
{
	// We wait for the GPU to finish command execution
	queue->Signal(queueFence.Get(), ++currentExecution); // will be final fence value, done after all GPU execution (++ ensures every other fence value is passed)
	WaitForFence(currentExecution);
}

bool D3D12Module::cleanUp()
{
	bool ok = true;
	if (drawEvent != nullptr)
		ok = CloseHandle(drawEvent);

	return ok;
}

bool D3D12Module::init()
{
	// REMEMBER TO CHANGE THIS SO THAT IT STOPS WHEN IT ENCOUNTERS AN ERROR (+ separate functions?)

	bool ok = true;

	// Enable debug layer (if debug mode)
#if defined (_DEBUG)
	ok = SUCCEEDED (D3D12GetDebugInterface( IID_PPV_ARGS(&debugInterface) ) );
	debugInterface->EnableDebugLayer();
#endif

	// 1. Device creation
	ok = ok and createDevice();

	// 2. Command queue set up

	D3D12_COMMAND_QUEUE_DESC queueDescription = {};
	queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // the same as the command lists used
	queueDescription.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDescription.NodeMask = 0; // single GPU operation
	ok = ok and SUCCEEDED (device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&queue)));

	// 3. Command allocators set up (one per frame buffer)

	for (unsigned int i = 0; ok and i < FRAMES_IN_FLIGHT; ++i)
		ok = ok and SUCCEEDED(device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]))); // same type as command lists

	// 4. Command list object creation
	ok = ok and SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)) ); // get() because that's how you get a ComPtr address; nullptr for now for a default pipeline state object
	ok = ok and SUCCEEDED(commandList->Close()); // so that in render() we can reset it (TO CHANGE LATER AND MAKE THE RESET IN postRender()?)

	// 5. Creating the render target view (RTV)

	// Swapchain set up (manages frame buffers)
	DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
	swapChainDescription.Width = winWidth;   // of the buffers,
	swapChainDescription.Height = winHeight; // in pixels

	swapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA format (8 bits per channel)
													   // UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0, which is what the screen understands)

	swapChainDescription.Stereo = FALSE; // no stereoscopic 3D rendering
	swapChainDescription.SampleDesc = { 1, 0 }; // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel; second then doesn't matter)
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Buffers will be used as render targets
	swapChainDescription.BufferCount = FRAMES_IN_FLIGHT; // (1 for the screen, the other for rendering)

	swapChainDescription.Scaling = DXGI_SCALING_STRETCH; // STRETCH = Stretch the image to fit the window
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern efficient swap method:
															  // - FLIP: Uses page flipping (no copying)
															  // - DISCARD: Discard previous back buffer contents

	swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
	swapChainDescription.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr <IDXGISwapChain1> swapChainOlderVer; // required for using function
	ok = ok and SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), hWnd, &swapChainDescription, nullptr, nullptr, &swapChainOlderVer) ); // first nullptd for windowed mode only; second to not restrict content to a certain output (monitor...)
	ok = ok and SUCCEEDED(swapChainOlderVer.As(&swapChain));

	currentFrameBuffIndex = swapChain->GetCurrentBackBufferIndex(); // for future reference

	// RTV Descriptor heap set up (descriptors for passing frame buffers to functions)
	// 1) Declaration
	D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
	heapDescription.NumDescriptors = FRAMES_IN_FLIGHT; // per frame buffer
	heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ok = ok and SUCCEEDED(device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&rtvDescriptorHeap)));

	// 2) Initialization
	if (ok) {
		UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // for stride between descriptors in heap
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); // (just a pointer!)

		for (unsigned int i = 0; ok and i < FRAMES_IN_FLIGHT; ++i) {
			ok = ok and SUCCEEDED(swapChain->GetBuffer(i, IID_PPV_ARGS(&frameBuffers[i])));
			if (ok) {

#if defined (_DEBUG)
				frameBuffers[i]->SetName(L"Backbuffer");
#endif
				device->CreateRenderTargetView(frameBuffers[i].Get(), nullptr, rtvHandle); // descriptor for buffer
				
				rtvHandle.ptr += rtvDescriptorSize;
			}
		}
	}
	
	// 6. Synchronisation objects set up
	ok = ok and SUCCEEDED (device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queueFence)) );
	if (ok){
		drawEvent = CreateEvent(nullptr, false, false, nullptr); // not inheritable, resets after thread released, not initially signaled, no name
		ok = ok and drawEvent != nullptr;
	}

		
	return ok;
}

void D3D12Module::preRender()
{

	commandAllocators[currentFrameBuffIndex]->Reset(); // empty commands (so that it doesn't fill up with previous data and can be used again; REQUIRES ASSIGNED CLOSED COMMAND LIST)
}

void D3D12Module::render()
{
	// 1. Reset command list so that accepts commands AND is associated to the current allocator
	commandList->Reset(commandAllocators[currentFrameBuffIndex].Get(), nullptr); // movable?

	// 2. Set frame buffer to render target state so we can add commands (CAN'T BE DONE ON A CLOSED COMMAND LIST!)
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(frameBuffers[currentFrameBuffIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	// 3. Add commands to the list
	// 1) Get handle (to buffer descriptor)
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // MAYBE BETTER HAVE A GLOBAL VARIABLE, SINCE WE CALL THIS ON init()?
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr += currentFrameBuffIndex * rtvDescriptorSize;

	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	// 2) Commands
	FLOAT color[4] = {1.f, 0.f, 0.f, 1.f};
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr); // nullptr so that it clears the entire view, not just a rectangle (0 was the number of rectangles)

	// 4. Set frame buffer to present state
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(frameBuffers[currentFrameBuffIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);

	// 5. Close command list (=> ready) and execute it	
	commandList->Close();
	ID3D12CommandList* listsToExecute[] = { commandList.Get() };
	queue->ExecuteCommandLists(_countof(listsToExecute),  listsToExecute);

	// 6. Present the frame (should change current back buffer)
	swapChain->Present(0, allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
}

void D3D12Module::postRender()
{
	queue->Signal(queueFence.Get(), ++currentExecution); // new fence value from the GPU side (not done until queue finishes previous commands)
	fenceValues[currentFrameBuffIndex] = currentExecution; // Store the fence for the dispatched buffer


	currentFrameBuffIndex = swapChain->GetCurrentBackBufferIndex();
	waitForGPU(); // will wait for next frame buffer to be available (currentFrameBuffIndex indicates the one)
}

// Auxiliary functions //

inline bool D3D12Module::createDevice()
{
	bool ok;


#if defined(_DEBUG)
	ok = SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));
#else
	ok = SUCCEEDED (CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)) );
#endif

	ComPtr<IDXGIAdapter4> adapter;
	ok = ok and SUCCEEDED (factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) ); // obtain adapter with best performance (0 to indicate the first with that criteria)
	ok = ok and SUCCEEDED (D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)) );

#if defined(_DEBUG) // add device break errors for particular events
	ComPtr<ID3D12InfoQueue> infoQueue;
	ok = ok and SUCCEEDED (device.As(&infoQueue) );
	ok = ok and SUCCEEDED (infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE) );
	ok = ok and SUCCEEDED (infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE) );
	ok = ok and SUCCEEDED (infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE) );
#endif

	return ok;
}

inline void D3D12Module::WaitForFence(UINT64 value)
{
	HRESULT hr = queueFence->SetEventOnCompletion(value, drawEvent);
	DWORD waitResult = WaitForSingleObject(drawEvent, INFINITE);
}

inline void D3D12Module::waitForGPU()
{
	if (queueFence->GetCompletedValue() < fenceValues[currentFrameBuffIndex])
		WaitForFence(fenceValues[currentFrameBuffIndex]);
}
