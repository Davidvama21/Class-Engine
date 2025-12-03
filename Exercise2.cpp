#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ModuleResources.h"
#include <ReadData.h>

#include "Exercise2.h"


bool Exercise2::init()
{
	if (not uploadVertexData()) return false;

	d3d12module = app->getD3D12Module();
	ID3D12Device5* device = d3d12module->getDevice();

	if (not createVertexSignature(device)) return false;

	// PIPELINE STATE DECLARATION //

	// 1. Assign signature to pipeline description
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();

	// 2. Add compiled shaders to pipeline description
	addCompiledShaders(psoDesc); //<- doesn't work???
	auto dataVS = DX::ReadData(L"Exercise2VS.cso");
	auto dataPS = DX::ReadData(L"Exercise2PS.cso");

	psoDesc.VS = { dataVS.data(), dataVS.size() };
	psoDesc.PS = { dataPS.data(), dataPS.size() };
	

	// 3. Set vertex shader variables layout
	// setVShaderLayout(psoDesc); // <- doesn't work either????
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	}; // 1st param is our arbitrary name, 2nd is a number to distinguish between vars with the same name, 3r is format, 4th is Input slot (number of buffer), 5th is offset of element inside the buffer

	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) }; // 2n param. is number of variables

	// 4. Remaining pipeline parameters
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // mesh topology
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of render targets
	psoDesc.NumRenderTargets = 1;
	psoDesc.SampleDesc = { 1, 0 };   // For multitasking
	psoDesc.SampleMask = 0xffffffff; // (to be seen)

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // Init.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);		      // with default values

	// 5. Finally, we create the pipeline object
	if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)))) return false;

	return true;
}

void Exercise2::render()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12module->getRenderTargetDescriptor();
	ID3D12GraphicsCommandList4* commandList = d3d12module->getCommandList();

	// Reset command list so that accepts commands AND is associated to the current allocator and pipeline (NOW WE USE THE PIPELINE)
	commandList->Reset(d3d12module->getCommandAllocator(), pipelineStateObject.Get());

	// Set frame buffer to render target state so we can add commands (CAN'T BE DONE ON A CLOSED COMMAND LIST!)
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12module->getBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);


	// Add commands to the list

	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr); // nullptr so that it clears the entire view, not just a rectangle (0 was the number of rectangles)


	// EXTRA COMMANDS (not seen in exercise 1 (more than painting the background)) //

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	// Set vertex buffer for rendering
	D3D12_VERTEX_BUFFER_VIEW pView; // for vertex buffer details
	pView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	pView.SizeInBytes = sizeof(vertices);
	pView.StrideInBytes = sizeof(Vertex); // stride between elements
	commandList->IASetVertexBuffers(0, 1, &pView); // 0 for device slot to be bound, 1 for number of vertex buffers <- you may want to ask about this to the teacher

	// Set viewport + scissor
	unsigned int windowWidth = d3d12module->getWindowWidth();
	unsigned int windowHeight = d3d12module->getWindowHeight();

	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(windowWidth), float(windowHeight) , 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, windowWidth, windowHeight };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	// Drawing
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0); // 3 vertices, 1 instance of them, vertices start at 0 and instances at 0
}


inline bool Exercise2::uploadVertexData()
{
	ComPtr<ID3D12Resource> uploadBuffer;
	ModuleResources* resModule = app->getModuleResources();
	if (not resModule->CreateUploadBuffer(vertices, sizeof(vertices), uploadBuffer, L"Vertex upload buffer")) return false;
	if (not resModule->CreateDefaultBuffer(uploadBuffer, sizeof(vertices), vertexBuffer, L"Vertex default buffer")) return false; // (on the GPU)
	return true;
}

inline bool Exercise2::createVertexSignature(ID3D12Device5* device)
{
	// Create empty root signature for the vertex shader
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // 3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT is because some pipelines do not use it (required for variable passing)

	ComPtr<ID3DBlob> blob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr))) // desc serialization (for signature creation)
		return false;

	if (FAILED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) return false;

	return true;
}

inline void Exercise2::addCompiledShaders(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	auto dataVS = DX::ReadData(L"Exercise2VS.cso");
	auto dataPS = DX::ReadData(L"Exercise2PS.cso");

	psoDesc.VS = { dataVS.data(), dataVS.size() };
	psoDesc.PS = { dataPS.data(), dataPS.size() };
}

inline void Exercise2::setVShaderLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	// Set vertex shader variables layout

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	}; // 1st param is our arbitrary name, 2nd is a number to distinguish between vars with the same name, 3r is format, 4th is Input slot (number of buffer), 5th is offset of element inside the buffer

	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) }; // 2n param. is number of variables
}
