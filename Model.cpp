#include "Globals.h"

// tinygltf config (ONLY IN ONE .CPP FILE)
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION /* Only in one of the includes */
#include "tiny_gltf.h"

#include "Model.h"

bool Model::load(const std::string& assetFileName, const std::string& assetFolder, BasicMaterial::Type materialType)
{
	tinygltf::TinyGLTF gltfContext;
	tinygltf::Model model;
	std::string error, warning;
	bool loadOk = gltfContext.LoadASCIIFromFile(&model, &error, &warning, assetFileName);
	if (loadOk)
	{
		// Load model implementation

		// 1. Calculate the number of meshes (primitives in tinygltf) and reserve its space
		int numMeshes = 0;
		for (unsigned int i = 0; i < model.meshes.size(); ++i)
			numMeshes += model.meshes[i].primitives.size();

		meshes.reserve(numMeshes);

		// 2. Reserve space for materials and potential texture descriptors (YOU MAY WANT TO CHANGE THIS, AND TRY TO CHECK FOR TEXTURES BEFORE RESERVING THE HEAP)
		materials.reserve(model.materials.size());
		descTable = ModuleShaderDescriptors(model.materials.size() + 1); // + 1 to reserve a null descriptor
		if (not descTable.init()) return false;

		descTable.createNullTexture2DSRV(); // we create a null descriptor at the top to use with materials without textures

		// 3. Traverse and collect meshes and materials

		for (unsigned int i = 0; i < model.meshes.size(); ++i) {

			for (unsigned int j = 0; j < model.meshes[i].primitives.size(); ++j) {
				Mesh mesh;
				if (not mesh.load(model, model.meshes[i], model.meshes[i].primitives[j]))
					return false;
				meshes.push_back(mesh);
			}
		}

		for (unsigned int i = 0; i < model.materials.size(); ++i) {
			BasicMaterial material;
			if (not material.load(model, model.materials[i], descTable, assetFolder, materialType))
				return false;
			materials.push_back(material);
		}

	}else {
		LOG("Error loading %s: %s", assetFileName.c_str(), error.c_str());
		return false;
	}

	return true;
}
