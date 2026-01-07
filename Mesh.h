#pragma once

#include <map>

namespace tinygltf {
	struct Model; 
	struct Mesh;
	struct Primitive;
};

struct Vertex
{
	Vector3 position;
	Vector2 texCoord0; // texture coordinates
};

class Mesh
{

public:

	// loads primitive data taken from a tinygltf model
	bool load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);

	void draw(ID3D12GraphicsCommandList* commandList) const;

	bool hasIndices() const {
		return indexBuffer != nullptr;
	}

	const std::string& getName() const {
		return name;
	}

	unsigned int getMaterialIndex() const {
		return materialIndex;
	}

private:

	std::string name;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // (so that we have the data readily available)
	uint32_t numVertices;

	ComPtr<ID3D12Resource> indexBuffer = nullptr; // nullptr will indicate that the primitive doesn't use indices
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	uint32_t numIndices;

	unsigned int materialIndex; // material of the gltf model

	// utility function that uses the second loadAccessorData 
	bool loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model, const std::map<std::string, int>& attributes, const char* accesorName);
	
	// Has:
	// - A generic buffer of bytes to store info (data)
	//	- The size of each element in bytes(elemSize, == tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type)!)
	//	- stride between elements (stride)
	//	- the total elements in the buffer(elemCount, == accessor.count!)
	bool loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model, int accesorIndex);
};

