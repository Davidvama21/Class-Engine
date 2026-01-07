#pragma once

#include "Module.h"

struct SHADER_RESOURCE_VIEW_DESC;

class ModuleShaderDescriptors : public Module
{
public:

	ModuleShaderDescriptors(unsigned int numSlots = SHADER_DESCRIPTORS);

	bool init();

	unsigned int CreateSRV (ComPtr<ID3D12Resource>& resource);
	unsigned int createNullTexture2DSRV(); // descriptor with no binded texture
	//unsigned int AllocateDescriptor(ComPtr<ID3D12Resource>& resource, SHADER_RESOURCE_VIEW_DESC description);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(unsigned int index) const {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			index, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(unsigned int index) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			index, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	ID3D12DescriptorHeap* getHeap() const { return shaderDescriptorHeap.Get(); }

	unsigned int getMaxSlots() { return numSlots; }

private:

	ComPtr<ID3D12DescriptorHeap> shaderDescriptorHeap; // for CBV + SRV + UAV descriptors
	unsigned int numSlots;
	unsigned int nextFreeSlot;

	ID3D12Device5* device; // for easy access
};

