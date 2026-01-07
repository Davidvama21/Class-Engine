#pragma once

#include <vector>
#include "ModuleShaderDescriptors.h"
#include "Mesh.h"
#include "BasicMaterial.h"

class Mesh;
class BasicMaterial;
class ModuleShaderDescriptors;

class Model
{
public:

	bool load(const std::string& assetFileName, const std::string& assetFolder);

	ID3D12DescriptorHeap* getHeap() const { return descTable.getHeap(); }

	ModuleShaderDescriptors& getDescTable() { return descTable;}

	const std::vector <Mesh>& getMeshes() const { return meshes; }
	const std::vector <BasicMaterial>& getMaterials() const { return materials; }

	unsigned int getNumMaterials() const { return materials.size(); }
private:

	std::vector <Mesh> meshes; // the primitives in tinygltf models
	std::vector <BasicMaterial> materials;

	ModuleShaderDescriptors descTable; // will be used for textures in materials
};

