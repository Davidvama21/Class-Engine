#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "Exercise1.h"

void Exercise1::render()
{
	D3D12Module* d3d12module = app->getD3D12Module();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12module->getRenderTargetDescriptor();
	ID3D12GraphicsCommandList4* commandList = d3d12module->getCommandList();

	// Add commands to the list

	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr); // nullptr so that it clears the entire view, not just a rectangle (0 was the number of rectangles)
}
