
#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"

#include "ModuleShaderDescriptors.h"

ModuleShaderDescriptors::ModuleShaderDescriptors(unsigned int numSlots)
{
	this->numSlots = numSlots;
}

bool ModuleShaderDescriptors::init() {

	D3D12Module* d3d12module = app->getD3D12Module();
	device = d3d12module->getDevice();

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numSlots; // basically space reserved
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
	if (nextFreeSlot == numSlots) // if the heap is full, return the index value, which is > max index
		return nextFreeSlot;

	D3D12_CPU_DESCRIPTOR_HANDLE SRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		nextFreeSlot, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(resource.Get(), nullptr, SRVHandle); // nullptr for default SRV descriptor

	return nextFreeSlot++; // we return the previous value, then update the free slot to the next one.
}

unsigned int ModuleShaderDescriptors::createNullTexture2DSRV()
{
	if (nextFreeSlot == numSlots) // if the heap is full, return the index value, which is > max index
		return nextFreeSlot;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Standard format
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE SRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		nextFreeSlot, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(nullptr, &srvDesc, SRVHandle); // nullptr for no resource binded to descriptor

	return nextFreeSlot++; // we return the previous value, then update the free slot to the next one.
}

/*unsigned int ModuleShaderDescriptors::AllocateDescriptor(ComPtr<ID3D12Resource>& resource, SHADER_RESOURCE_VIEW_DESC description)
{
	if (nextFreeSlot == numSlots) // if the heap is full, return the index value, which is > max index
		return nextFreeSlot;

	D3D12_CPU_DESCRIPTOR_HANDLE SRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		nextFreeSlot, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(resource.Get(), description, SRVHandle); // nullptr for default SRV descriptor

	return nextFreeSlot++; // we return the previous value, then update the free slot to the next one.
}
*/


