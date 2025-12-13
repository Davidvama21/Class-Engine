#pragma once

#include "Module.h"

class ModuleSampler : public Module
{
public:

	// Type that will be the index of each sampler (and MAX_SAMPLERS will be the total samplers we have specified)
	enum Type
	{
		LINEAR_WRAP,
		MAX_SAMPLERS
	};


	bool init() override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(Type type) {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(samplerHeap->GetCPUDescriptorHandleForHeapStart(), type,
			samplerDescriptorSize);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(Type type) {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(samplerHeap->GetGPUDescriptorHandleForHeapStart(), type,
			samplerDescriptorSize);
	}

	inline ID3D12DescriptorHeap* getHeap() const { return samplerHeap.Get(); }

private:

	ComPtr<ID3D12DescriptorHeap> samplerHeap;
	UINT samplerDescriptorSize;

	void CreateDefaultSamplers(ID3D12Device5* device);
};

