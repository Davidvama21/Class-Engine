#pragma once

class ModuleShaderDescriptors;

namespace tinygltf {
	struct Model;
	struct Material;
};

struct MaterialData
{
	Vector4 baseColour;
	BOOL hasColourTexture; // use BOOL (4 bytes) instead of c++ bool (1 byte) as HLSL bool is 4 bytes long
};

class BasicMaterial
{
public:

	bool load(const tinygltf::Model& model, const tinygltf::Material& material, ModuleShaderDescriptors& descTable, const std::string& basePath);

	D3D12_GPU_VIRTUAL_ADDRESS getMatGPUVirtualAddress() const {
		return materialBuffer->GetGPUVirtualAddress();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(const ModuleShaderDescriptors& descTable) const {
		return descTable.GetCPUHandle(descriptorIndex);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(const ModuleShaderDescriptors& descTable) const {
		return descTable.GetGPUHandle(descriptorIndex);
	}


private:

	std::string name;

	ComPtr<ID3D12Resource> materialBuffer;

	ComPtr<ID3D12Resource> textureBuffer;
	unsigned int descriptorIndex = 0; // will be = 0 if no texture (points to a null descriptor)
};

