
#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"

#include "ModuleShaderDescriptors.h"

bool ModuleShaderDescriptors::init() {

	D3D12Module* d3d12module = app->getD3D12Module();
	device = d3d12module->getDevice();

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = SHADER_DESCRIPTORS; // basically space reserved
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&shaderDescriptorHeap)))) {
		nextFreeSlot = 0; // YOU CAN ALSO DO THIS ON THE CONSTRUCTOR (MAYBE BETTER THAN THIS)

		shaderDescriptorHeap->SetName(L"CBV SRV UAV Resources Heap");

		return true;
	}

	return false;
}

unsigned int ModuleShaderDescriptors::CreateSRV(ComPtr<ID3D12Resource>& resource)
{
	if (nextFreeSlot == SHADER_DESCRIPTORS) // if the heap is full, return the index value, which is > max index
		return nextFreeSlot;

	D3D12_CPU_DESCRIPTOR_HANDLE SRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		nextFreeSlot, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(resource.Get(), nullptr, SRVHandle); // nullptr for default SRV descriptor

	return nextFreeSlot++; // we return the previous value, then update the free slot to the next one.
}

/*unsigned int ModuleShaderDescriptors::AllocateDescriptor(ComPtr<ID3D12Resource>& resource, SHADER_RESOURCE_VIEW_DESC description)
{
	if (nextFreeSlot == SHADER_DESCRIPTORS) // if the heap is full, return the index value, which is > max index
		return nextFreeSlot;

	D3D12_CPU_DESCRIPTOR_HANDLE SRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		nextFreeSlot, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(resource.Get(), description, SRVHandle); // nullptr for default SRV descriptor

	return nextFreeSlot++; // we return the previous value, then update the free slot to the next one.
}
*/


