#include "Globals.h"

#include "tiny_gltf.h"
#include "ModuleResources.h"
#include "Application.h"

#include "Mesh.h"

bool Mesh::load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive)
{
	name = mesh.name;

	const auto& itPos = primitive.attributes.find("POSITION");
	if (itPos != primitive.attributes.end()) // If no position no geometry data
	{
		numVertices = uint32_t(model.accessors[itPos->second].count);

		std::unique_ptr <Vertex[]> vertices(new Vertex[numVertices]);
		uint8_t* vertexData = reinterpret_cast<uint8_t*>(vertices.get()); // Casts Vertex Buffer to Bytes (uint8_t*) buffer

		if (not loadAccessorData(vertexData + offsetof(Vertex, position), sizeof(Vector3), sizeof(Vertex),
			numVertices, model, itPos->second)) // load position
			return false;
		if (not loadAccessorData(vertexData + offsetof(Vertex, texCoord0), sizeof(Vector2), sizeof(Vertex),
			numVertices, model, primitive.attributes, "TEXCOORD_0")) // load texture coordinate
			return false;

		// Copy Vertex data to default buffer
		uint32_t verticesSize = numVertices * sizeof(Vertex); 
		{
		ComPtr<ID3D12Resource> uploadBuffer;
		if (not app->getModuleResources()->CreateUploadBuffer(vertices.get(), verticesSize, uploadBuffer, "Vertex upload buffer")) return false;
		if (not app->getModuleResources()->CreateDefaultBuffer(uploadBuffer, verticesSize, vertexBuffer, name.c_str())) return false;
		}

		// Vertex Buffer View setup
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = verticesSize;

		if (primitive.indices >= 0) { // if we have indices
			tinygltf::Accessor indAccessor = model.accessors[primitive.indices];

			if (indAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT ||
				indAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT ||
				indAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE) // ONLY SUPPORTED FORMATS! 
			{
				uint32_t indexElementSize = tinygltf::GetComponentSizeInBytes(indAccessor.componentType);
				numIndices = uint32_t(indAccessor.count);
				unsigned int indicesSize = numIndices * indexElementSize; // YOU MAY WANT TO SAVE THIS ON THE MESH CLASS 
				std::unique_ptr<uint8_t[]> indices(new uint8_t[indicesSize]);
				if (not loadAccessorData(indices.get(), indexElementSize, indexElementSize, numIndices, model, primitive.indices))
					return false;

				// Copy Index data to default buffer
				ComPtr<ID3D12Resource> uploadBuffer;
				if (not app->getModuleResources()->CreateUploadBuffer(indices.get(), indicesSize, uploadBuffer, "Index upload buffer")) return false;
				if (not app->getModuleResources()->CreateDefaultBuffer(uploadBuffer, indicesSize, indexBuffer, name.c_str())) return false;

				// Index Buffer View setup
				static const DXGI_FORMAT formats[3] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT };

				indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
				indexBufferView.Format = formats[indexElementSize >> 1];
				indexBufferView.SizeInBytes = numIndices * indexElementSize;
			}
		}

		materialIndex = primitive.material;
	}
	return true;
}

void Mesh::draw(ID3D12GraphicsCommandList* commandList) const
{
	//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	if (hasIndices())
	{
		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0); //  draw 1 instance of them, indices start at 0, with value added 0 and instances starting at 0
	}
	else
	{
		commandList->DrawInstanced(numVertices, 1, 0, 0);  // draw 1 instance of them, vertices start at 0 and instances at 0
	}
}


bool Mesh::loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model, const std::map<std::string, int>& attributes, const char* accesorName)
{
	const auto& it = attributes.find(accesorName);
	if (it != attributes.end())
	{
		return loadAccessorData(data, elemSize, stride, elemCount, model, it->second);
	}

	return false;
}

bool Mesh::loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model, int accesorIndex)
{
	tinygltf::Accessor accessor = model.accessors[accesorIndex];

	if (elemCount != accessor.count or elemSize != tinygltf::GetComponentSizeInBytes(accessor.componentType) *
		tinygltf::GetNumComponentsInType(accessor.type))
	return false;

	tinygltf::BufferView bufferView = model.bufferViews[accessor.bufferView];

	const uint8_t* bufferCopyPoint = reinterpret_cast<const uint8_t*>(&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);

	size_t bufferStride; // distance between elements (important, because we may have interleaved data)
	if (bufferView.byteStride != 0) bufferStride = bufferView.byteStride;
	else bufferStride = elemSize; // distance when no interleaved elements

	while (elemCount > 0) {

		memcpy(data, bufferCopyPoint, elemSize);

		data += stride; // advance in destiny buffer
		bufferCopyPoint += bufferStride; // advance in model buffer

		--elemCount;
	}

	return true;
}
