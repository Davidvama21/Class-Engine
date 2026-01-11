#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "EditorModule.h"
#include "ModuleResources.h"
#include "ModuleCamera.h"
#include "ModuleSampler.h"
#include <ReadData.h>

#include "Exercise5.h"

bool Exercise5::init()
{
	ModuleResources* resourcesModule = app->getModuleResources();

	if (not loadModelData(resourcesModule)) return false;

	d3d12Module = app->getD3D12Module();
	ID3D12Device5* device = d3d12Module->getDevice();

	if (not createVertexSignature(device)) return false;

	if (not createPipelineStateObject(device)) return false;

	samplerModule = app->getModuleSampler();
	editorModule = app->getEditorModule();
	cameraModule = app->getModuleCamera();

	debugDraw = std::unique_ptr<DebugDrawPass>(new DebugDrawPass(device, d3d12Module->getCommandQueue()));

	return true;
}

void Exercise5::render()
{
	setupMVP();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12Module->getRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = d3d12Module->getDepthStencilDescriptor();
	ID3D12GraphicsCommandList4* commandList = d3d12Module->getCommandList();

	// Reset command list so that accepts commands AND is associated to the current allocator and pipeline (NOW WE USE THE PIPELINE)
	commandList->Reset(d3d12Module->getCommandAllocator(), pipelineStateObject.Get());

	// Set frame buffer to render target state so we can add commands (CAN'T BE DONE ON A CLOSED COMMAND LIST!)
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12Module->getBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);


	// Add commands to the list
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle); // now we have an actual depth stencil, so we have to pass it

	float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); // nullptr so that it clears the entire view, not just a rectangle (0 was the number of rectangles)

	// EXTRA COMMANDS (not seen in exercise 1 (more than painting the background)) //

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr); // 1.0 for clear depth value, 0 for clear stencil value, 0 for specified rectangles to clear (if 0 => nullptr for rectangle list) 

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	// Pass the mpv, which will be inserted in the root signature
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0); // first 0 because parameter 0

	// Assign descriptor heaps for textures and samplers
	ID3D12DescriptorHeap* descriptorHeaps[] = { model.getHeap(), samplerModule->getHeap() };
	commandList->SetDescriptorHeaps(2, descriptorHeaps);

	commandList->SetGraphicsRootDescriptorTable(3, samplerModule->GetGPUHandle(ModuleSampler::Type(editorModule->samplerType()))); // set sampler handle (REMEMBER THAT THE NUMBER IS THE ROOT SIGNATURE POSITION!)

	// Set viewport + scissor
	unsigned int windowWidth = d3d12Module->getWindowWidth();
	unsigned int windowHeight = d3d12Module->getWindowHeight();

	D3D12_VIEWPORT viewport = getViewport(windowWidth, windowHeight);
	D3D12_RECT scissor = getScissorRect(windowWidth, windowHeight);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	// Drawing
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	BEGIN_EVENT(commandList, "Model Render Pass");

	for (const Mesh& mesh : model.getMeshes())
	{
		const BasicMaterial* material = model.getMaterials()[mesh.getMaterialIndex()].get();
		commandList->SetGraphicsRootConstantBufferView(1, materialBuffers[mesh.getMaterialIndex()]->GetGPUVirtualAddress() ); // set cbuffer handle (from the material used by the mesh)
		commandList->SetGraphicsRootDescriptorTable(2, material->getGPUHandle(model.getDescTable()) ); // set texture handle (same)

		mesh.draw(commandList);
	}

	END_EVENT(commandList);

	// Debug elements (grid, arrows...)

	if (editorModule->gridEnabled()) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray); // Grid plane
	if (editorModule->objectAxisEnabled()) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f); // XYZ axis

	debugDraw->record(commandList, windowWidth, windowHeight, view, projection);
}


inline bool Exercise5::loadModelData(ModuleResources* resourcesModule)
{
	if (not model.load(modelPath, modelFolder)) return false;

	// Upload material data
	materialBuffers.reserve(model.getNumMaterials());
	for (int i = 0, count = model.getNumMaterials(); i < count; ++i) {

		const BasicMaterial* material = model.getMaterials()[i].get();
		BasicMaterialData data = material->getBasicData();
		
		ComPtr<ID3D12Resource> uploadBuffer;
		ComPtr<ID3D12Resource> materialBuffer;

		if (not resourcesModule->CreateUploadBuffer(&data, alignUp(sizeof(BasicMaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), uploadBuffer, "Material upload buffer")) return false;
		if (not resourcesModule->CreateDefaultBuffer(uploadBuffer, alignUp(sizeof(BasicMaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), materialBuffer, material->getName().c_str())) return false;

		materialBuffers.push_back(materialBuffer);
	}
	return true;
}

inline bool Exercise5::createVertexSignature(ID3D12Device5* device)
{
	// Create root signature with mvp + texture + sampler object on it for the vertex shader

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	CD3DX12_ROOT_PARAMETER rootParameters[4] = {};

	// mvp //
	rootParameters[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0); // number of 32 bit elements in a matrix (second param is shader register index)

	// cbuffer //
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_PIXEL); // (shader register 1)

	// texture //
	CD3DX12_DESCRIPTOR_RANGE tableRange;
	tableRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0); // 1 range of 1 SRV descriptor, on register 0
	rootParameters[2].InitAsDescriptorTable(1, &tableRange, D3D12_SHADER_VISIBILITY_PIXEL); // The descriptor table (1 range, specified by tableRange, visible on pixel shader)

	// sampler //
	CD3DX12_DESCRIPTOR_RANGE sampRange;
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSampler::Type::MAX_SAMPLERS, 0);
	rootParameters[3].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignatureDesc.Init(UINT(std::size(rootParameters)), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // 0 for no static samplers (=> nullptr afterwards for sampler list)

	ComPtr<ID3DBlob> blob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr))) // desc serialization (for signature creation)
		return false;

	if (FAILED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) // 0 when no multiple GPU operation
		return false;

	return true;
}

inline bool Exercise5::createPipelineStateObject(ID3D12Device5* device)
{
	// 1. Assign signature to pipeline description
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();

	// 2. Add compiled shaders to pipeline description
	std::vector<uint8_t> dataVS, dataPS;
	getCompiledShaders(dataVS, dataPS);
	psoDesc.VS = { dataVS.data(), dataVS.size() };
	psoDesc.PS = { dataPS.data(), dataPS.size() };

	// 3. Set vertex shader variables layout (on the shader, how they are)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	}; // 1st param is our arbitrary name, 2nd is a number to distinguish between vars with the same name, 3r is format, 4th is Input slot (number of buffer), 5th is offset of element inside the buffer (D3D12_APPEND_ALIGNED_ELEMENT calculates it auto. based on previous variable)

	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) }; // 2n param. is number of variables

	// 4. Remaining pipeline parameters
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // mesh topology
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of render targets
	psoDesc.NumRenderTargets = 1;
	psoDesc.SampleDesc = { 1, 0 };   // For multitasking
	psoDesc.SampleMask = 0xffffffff; // (to be seen)

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true; // because that's the vertex order for our glTF faces
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	// DebugDraw-related parameters (depth stencil stuff)
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	// 5. Finally, we create the pipeline object
	if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)))) return false;

	return true;
}

inline void Exercise5::getCompiledShaders(std::vector<uint8_t>& VS, std::vector<uint8_t>& PS)
{
	VS = DX::ReadData(L"Exercise4VS.cso");
	PS = DX::ReadData(L"Exercise5PS.cso");
}

inline void Exercise5::setupMVP()
{
	Matrix model = Matrix::CreateScale(modelScale, modelScale, modelScale);
	view = cameraModule->getViewMatrix();
	projection = cameraModule->getProjectionMatrix();

	mvp = (model * view * projection).Transpose(); // transpose because the shader only accepts column-major matrices
}
