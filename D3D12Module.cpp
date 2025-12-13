#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd): hWnd (hwnd), currentExecution(0)
{
}

D3D12Module::~D3D12Module() // REMEMBER THAT WE HAVE A CLEANUP() FUNCTION!
{
	// We wait for the GPU to finish command execution
    flush();
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
#if defined(_DEBUG)
	enableDebugLayer();
#endif
    
    bool ok = getWindowSize(winWidth, winHeight);

    ok = ok && createFactory();
	ok = ok && createDevice(false);

#if defined(_DEBUG)
	ok = ok && setupInfoQueue();
#endif 
	ok = ok && createDrawCommandQueue();

	ok = ok && createSwapChain();
	ok = ok && createRenderTargets();
    ok = ok && createDepthStencil();
	ok = ok && createCommandList();
	ok = ok && createDrawFence();

	if (ok)
	{
        currentFrameBuffIndex = swapChain->GetCurrentBackBufferIndex();
	}

	return ok;

}

void D3D12Module::enableDebugLayer()
{
    ComPtr<ID3D12Debug> debugInterface;

    D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));

    debugInterface->EnableDebugLayer();
}

bool D3D12Module::createFactory()
{
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    return SUCCEEDED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));
}

bool D3D12Module::createDevice(bool useWarp)
{
    bool ok = true;

    if (useWarp)
    {
        ComPtr<IDXGIAdapter1> adapter;
        ok = ok && SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
        ok = ok && SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
    }
    else
    {
        ComPtr<IDXGIAdapter4> adapter;
        SIZE_T maxDedicatedVideoMemory = 0;
        factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
        ok = SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
    }

    if (ok)
    {
        // Check for tearing to disable V-Sync needed for some monitors
        BOOL tearing = FALSE;
        factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing));

        allowTearing = tearing == TRUE;

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
        HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
        if (SUCCEEDED(hr) && features5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            supportsRT = true;
        }
    }

    return ok;
}

bool D3D12Module::setupInfoQueue()
{
    ComPtr<ID3D12InfoQueue> pInfoQueue;

    bool ok = SUCCEEDED(device.As(&pInfoQueue));
    if (ok)
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        //D3D12_MESSAGE_SEVERITY Severities[] =
        //{
            //D3D12_MESSAGE_SEVERITY_INFO
        //};

        // Suppress individual messages by their ID
        //D3D12_MESSAGE_ID DenyIds[] = {
            //D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            //D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            //D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        //};

        //D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        //NewFilter.DenyList.NumSeverities = _countof(Severities);
        //NewFilter.DenyList.pSeverityList = Severities;
        //NewFilter.DenyList.NumIDs = _countof(DenyIds);
        //NewFilter.DenyList.pIDList = DenyIds;

        //ok = SUCCEEDED(pInfoQueue->PushStorageFilter(&NewFilter));
    }

    return ok;
}

bool D3D12Module::createDrawCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    return SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)));
}

bool D3D12Module::createSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = winWidth;
    swapChainDesc.Height = winHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = FRAMES_IN_FLIGHT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1>     swapChain1;
    bool ok = SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

    ok = ok && SUCCEEDED(swapChain1.As(&swapChain));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ok = SUCCEEDED(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

    return ok;
}

bool D3D12Module::createRenderTargets()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = FRAMES_IN_FLIGHT;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    bool ok = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap)));

    if (ok)
    {
        UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        for (int i = 0; ok && i < FRAMES_IN_FLIGHT; ++i)
        {
            ok = SUCCEEDED(swapChain->GetBuffer(i, IID_PPV_ARGS(&frameBuffers[i])));

            if (ok)
            {
                frameBuffers[i]->SetName(L"BackBuffer");

                device->CreateRenderTargetView(frameBuffers[i].Get(), nullptr, rtvHandle);
            }

            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    return ok;
}

bool D3D12Module::createDepthStencil()
{
    // 1. Create depth/stencil texture

    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT); // default heap for quick access
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, winWidth, winHeight,
        1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL); // as a 2d texture that for now will only cover depth values (DXGI_FORMAT_D32_FLOAT), that tackles all the window, and allows creating a depth stencil view

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f; // corresponds to the far plane in Normalized Device Coordinates
    clearValue.DepthStencil.Stencil = 0;

    bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
        IID_PPV_ARGS(&depthStencilBuffer)));

    if (ok) {
        depthStencilBuffer->SetName(L"Depth/Stencil Texture");

        // 2. Create depth/stencil view (DSV)
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1; // only for the depth/stencil
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ok = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvDescriptorHeap)));

        if (ok) {
            dsvDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

            device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); // nullptr for a default descriptor set up
        }
    }

    return ok;
}

bool D3D12Module::createCommandList()
{
    bool ok = true;

    for (unsigned i = 0; ok && i < FRAMES_IN_FLIGHT; ++i)
    {
        ok = SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])));
    }

    ok = ok && SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)));
    ok = ok && SUCCEEDED(commandList->Close());

    return ok;
}

bool D3D12Module::createDrawFence()
{
    bool ok = SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queueFence)));

    if (ok)
    {
        drawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        ok = drawEvent != NULL;
    }

    return ok;
}

inline bool D3D12Module::getWindowSize(unsigned int& width, unsigned int& height)
{
    RECT rectangle;
    if (GetClientRect(hWnd, &rectangle) ) {

        width = rectangle.right - rectangle.left;
        height = rectangle.bottom - rectangle.top;

        return true;
    }

    return false;
}

void D3D12Module::preRender()
{

    currentFrameBuffIndex = swapChain->GetCurrentBackBufferIndex();

    if (fenceValues[currentFrameBuffIndex] != 0)
        WaitForFence(fenceValues[currentFrameBuffIndex]);

    // Empty commands (so that it doesn't fill up with previous data and can be used again; REQUIRES ASSIGNED CLOSED COMMAND LIST)
    commandAllocators[currentFrameBuffIndex]->Reset(); 
}

void D3D12Module::render()
{
	
}

void D3D12Module::postRender()
{
    // Set frame buffer to present state
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(frameBuffers[currentFrameBuffIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    // Close command list (=> ready) and execute it	
    if (SUCCEEDED(commandList->Close())) {

        ID3D12CommandList* listsToExecute[] = { commandList.Get() };
        queue->ExecuteCommandLists(UINT(std::size(listsToExecute)), listsToExecute);
    }

    // Present the frame (should change current back buffer)
    swapChain->Present(0, allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);

    queue->Signal(queueFence.Get(), ++currentExecution); // new fence value from the GPU side (not done until queue finishes previous commands)
    fenceValues[currentFrameBuffIndex] = currentExecution; // Store the fence for the dispatched buffer
}

void D3D12Module::resize()
{
    unsigned int width, height;
    bool ok = getWindowSize(width, height);

    if (ok and (width != winWidth or height != winHeight)) {
        winWidth = width;
        winHeight = height;

        if (winWidth > 0 and winHeight > 0) { // Doing something on 0 cases could give issues, + visualization doesn't matter then
            
            flush(); // wait so that we can touch the buffers
            
            // 1. Clear frame buffer references (since frame sizes will change)
            for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
            {
                frameBuffers[i].Reset();
                fenceValues[i] = 0; // so that we don't wait on next render() calls
            }

            // 2. Resize frame buffers
            DXGI_SWAP_CHAIN_DESC pDesc;
            ok = ok and SUCCEEDED(swapChain->GetDesc(&pDesc));

            ok = ok and SUCCEEDED(swapChain->ResizeBuffers(0, winWidth, winHeight, DXGI_FORMAT_UNKNOWN, pDesc.Flags)); // 0 to preserve number of frame buffers, DXGI_FORMAT_UNKNOWN to keep the format

            // 3. Update frame descriptors and references
            ok = ok and createRenderTargets();

            // 4. Update depth/stencil buffer
            ok = ok and createDepthStencil();
        }
    }
}

// Auxiliary functions //

inline void D3D12Module::WaitForFence(UINT64 value)
{
	HRESULT hr = queueFence->SetEventOnCompletion(value, drawEvent);
	DWORD waitResult = WaitForSingleObject(drawEvent, INFINITE);
}

// active wait for GPU things to finish (YOU MAY WANT TO CHANGE NAME OF currentExecution)
inline void D3D12Module::flush() { 

    queue->Signal(queueFence.Get(), ++currentExecution);
    WaitForFence(currentExecution);
}
