#pragma once

class ModuleShaderDescriptors;

namespace tinygltf {
	struct Model;
	struct Material;
};

struct BasicMaterialData
{
	Vector4 baseColour;
	BOOL hasColourTexture; // use BOOL (4 bytes) instead of c++ bool (1 byte) as HLSL bool is 4 bytes long
};

struct PhongMaterialData // BasicMaterialData with added Phong lighting parameters
{
	XMFLOAT4 diffuseColour;
	float    Kd;
	float    Ks;
	float    shininess;
	BOOL     hasDiffuseTex;
};

class BasicMaterial
{
public:

	enum Type
	{
		BASIC = 0,
		PHONG,
		PBR_PHONG
	};

	bool load(const tinygltf::Model& model, const tinygltf::Material& material, ModuleShaderDescriptors& descTable, const std::string& basePath, Type materialType);

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(const ModuleShaderDescriptors& descTable) const {
		return descTable.GetCPUHandle(descriptorIndex);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(const ModuleShaderDescriptors& descTable) const {
		return descTable.GetGPUHandle(descriptorIndex);
	}

	// Make sure you have the right type!
	//const BasicMaterialData* getBasicData() const { return basicData.get(); }
    //const PhongMaterialData* getPhongData() const { return phongData.get(); }
	BasicMaterialData getBasicData() const {
		
		BasicMaterialData basicData;

		basicData.baseColour = baseColour;
		basicData.hasColourTexture = hasColourTexture;
		
		return basicData; 
	}

	PhongMaterialData getPhongData() const {
		
		PhongMaterialData phongData;

		phongData.diffuseColour = baseColour;
		phongData.hasDiffuseTex = hasColourTexture;

		phongData.Kd = Kd;
		phongData.Ks = Ks;
		phongData.shininess = shininess;

		return phongData; 
	
	}



	Type getType() const { return type; }
	const std::string& getName() const { return name; }


private:

	std::string name;
	Type type = BASIC;

	// Common material parameters
	Vector4 baseColour;
	BOOL hasColourTexture = false;

	// Phong parameters
	float    Kd;
	float    Ks;
	float    shininess;

	//ComPtr<ID3D12Resource> materialBuffer;

	ComPtr<ID3D12Resource> textureBuffer;
	unsigned int descriptorIndex = 0; // will be = 0 if no texture (points to a null descriptor)
};

