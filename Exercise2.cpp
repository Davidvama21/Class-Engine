#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ModuleResources.h"
#include "Exercise2.h"

bool Exercise2::init()
{
	bool ok;

	// 1. Upload the vertex data to the GPU
	ComPtr<ID3D12Resource> uploadBuffer;
	ok = (app->getModuleResources())->CreateUploadBuffer(vertices, sizeof(vertices), uploadBuffer, L"Vertex upload buffer");
	ok = ok && (app->getModuleResources())->CreateDefaultBuffer(uploadBuffer, sizeof(vertices), vertexBuffer, L"Vertex default buffer");
	

	return ok;
}

void Exercise2::render()
{

	D3D12Module* d3d12module = app->getD3D12Module();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12module->getRenderTargetDescriptor();
	ID3D12GraphicsCommandList4* commandList = d3d12module->getCommandList();

	// Add commands to the list

	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr); // nullptr so that it clears the entire view, not just a rectangle (0 was the number of rectangles)

	// The rest goes here...
}
