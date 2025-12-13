#pragma once

#include "Module.h"

struct SHADER_RESOURCE_VIEW_DESC;

class ModuleShaderDescriptors : public Module
{
public:

	bool init();

	unsigned int CreateSRV (ComPtr<ID3D12Resource>& resource);
	//unsigned int AllocateDescriptor(ComPtr<ID3D12Resource>& resource, SHADER_RESOURCE_VIEW_DESC description);

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(unsigned int index) const {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			index, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(unsigned int index) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(shaderDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			index, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	inline ID3D12DescriptorHeap* getHeap() const { return shaderDescriptorHeap.Get(); }

private:

	ComPtr<ID3D12DescriptorHeap> shaderDescriptorHeap; // for CBV + SRV + UAV descriptors
	unsigned int nextFreeSlot;

	ID3D12Device5* device; // for easy access
};

