#include "Globals.h"

#include "tiny_gltf.h"

#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"
#include "Application.h"

#include "BasicMaterial.h"

bool BasicMaterial::load(const tinygltf::Model& model, const tinygltf::Material& material, ModuleShaderDescriptors& descTable, const std::string& basePath)
{
	name = material.name;

	MaterialData mat;

	mat.baseColour = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]), float(material.pbrMetallicRoughness.baseColorFactor[1]),
		float(material.pbrMetallicRoughness.baseColorFactor[2]), float(material.pbrMetallicRoughness.baseColorFactor[3]));

	if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) // there is a texture
	{
		const tinygltf::Texture& texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
		const tinygltf::Image& image = model.images[texture.source];

		mat.hasColourTexture = !image.uri.empty(); // there is a path to the texture
		if (mat.hasColourTexture)
		{
			if (not app->getModuleResources()->createTextureFromFile(basePath + image.uri, textureBuffer)) // the uri is usually a local path, so concatenate with model folder path
				return false;

			descriptorIndex = descTable.CreateSRV(textureBuffer); // descriptor creation
			if (descriptorIndex == descTable.getMaxSlots())
				return false;
		}

	}
	else mat.hasColourTexture = false;

	// Copy Material data to default buffer
	ComPtr<ID3D12Resource> uploadBuffer;
	// (the alignment with these buffers is required, since the default buffer will be passed to a cbuffer in HLSL Shader)
	if (not app->getModuleResources()->CreateUploadBuffer(&mat, alignUp(sizeof(MaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), uploadBuffer, "Material upload buffer")) return false;
	if (not app->getModuleResources()->CreateDefaultBuffer(uploadBuffer, alignUp(sizeof(MaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), materialBuffer, name.c_str())) return false;

	return true;
}
