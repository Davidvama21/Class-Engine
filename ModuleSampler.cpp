#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"

#include "ModuleSampler.h"

bool ModuleSampler::init() {

	ID3D12Device5* device = app->getD3D12Module()->getDevice();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MAX_SAMPLERS;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	bool ok = SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&samplerHeap)) ); // Create Sampler Descriptor Heap
	
	if (ok){
		samplerHeap->SetName(L"Shader Descriptors Heap");
		
		CreateDefaultSamplers(device); // Create default samplers (e.g., Linear Wrap, Point Clamp)
	}	

	return ok;
}

void ModuleSampler::CreateDefaultSamplers(ID3D12Device5* device)
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = samplerHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_SAMPLER_DESC samplers[MAX_SAMPLERS] = {
		// 1. Linear Wrap Sampler
		{
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // Linear filtering for min, mag, and mip
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // Wrap addressing mode
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f, // Mip LOD bias
		1, // Max anisotropy
		D3D12_COMPARISON_FUNC_ALWAYS, // No comparison
		{ 0, 0, 0, 0 }, // Border color (not used)
		0.0f, D3D12_FLOAT32_MAX // Min and Max LOD
		},
		
		// 2. Point Clamp Sampler
		//. . .
	};

	for (int i = 0; i < std::size(samplers); i++) {
		device->CreateSampler(&samplers[i], cpuHandle);
		cpuHandle.ptr += samplerDescriptorSize; // Move to the next descriptor slot in the heap
	}
}
