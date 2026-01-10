#include "Globals.h"

#include "tiny_gltf.h"

#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"
#include "Application.h"

#include "BasicMaterial.h"

bool BasicMaterial::load(const tinygltf::Model& model, const tinygltf::Material& material, ModuleShaderDescriptors& descTable, const std::string& basePath, Type materialType)
{
	name = material.name;
	type = materialType;

	// 1. Initialise common data

	baseColour = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]), float(material.pbrMetallicRoughness.baseColorFactor[1]),
		float(material.pbrMetallicRoughness.baseColorFactor[2]), float(material.pbrMetallicRoughness.baseColorFactor[3]));
	
	if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) // there is a texture
	{
		const tinygltf::Texture& texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
		const tinygltf::Image& image = model.images[texture.source];

		hasColourTexture = !image.uri.empty(); // there is a path to the texture
		if (hasColourTexture)
		{
			if (not app->getModuleResources()->createTextureFromFile(basePath + image.uri, textureBuffer)) // the uri is usually a local path, so concatenate with model folder path
				return false;

			descriptorIndex = descTable.CreateSRV(textureBuffer); // descriptor creation
			if (descriptorIndex == descTable.getMaxSlots())
				return false;
		}

	}
	else hasColourTexture = false;

	// 2. Initialise not common data

	switch (materialType) {

	case PHONG:

		// Modifiable?
		Kd = 0.85f;
		Ks = 0.35f;
		shininess = 32.0f;
	}

	return true;
}
