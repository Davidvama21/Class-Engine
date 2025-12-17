#pragma once

#include "Module.h"

#include "DebugDrawPass.h"

class Exercise4 : public Module
{
public:

	bool init();

	virtual void render() override;

private:

	struct Vertex
	{
		Vector3 position;
		Vector2 uv; // texture coordinates
	};

	Vertex vertices[6] =
	{
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(-0.2f, -0.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.2f, 1.2f) }
	};

	Matrix mvp; // will contain transformations for vertices
	Matrix projection, view;
	
	//std::filesystem::path texturePath = L"Assets/Textures/dog.dds";
	std::filesystem::path texturePath = L"Assets/Textures/cracked_ground.jpg";

	std::unique_ptr <DebugDrawPass> debugDraw; // for grid, object arrows

	// For easy access
	D3D12Module* d3d12Module;
	EditorModule* editorModule;
	ModuleCamera* cameraModule;
	ModuleShaderDescriptors* shaderDescModule; // YOU MAY WANT TO SET THE SIZE FOR THIS OPTIMALLY
	ModuleSampler* samplerModule;

	// Pipeline related objects //
	ComPtr<ID3D12Resource> vertexBuffer; // will contain vertex data on the GPU (is a default buffer)
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders (to indicate passed paramateres)

	ComPtr<ID3D12Resource> texture; // will also be on the GPU
	unsigned int textureHeapIndex;

	ComPtr<ID3D12PipelineState> pipelineStateObject;

	inline bool uploadVertexData(ModuleResources* resModule);
	inline bool uploadTextureData(ModuleResources* resModule);
	inline bool createVertexSignature(ID3D12Device5* device);
	inline bool createPipelineStateObject(ID3D12Device5* device);

	inline void getCompiledShaders(std::vector<uint8_t>& VS, std::vector<uint8_t>& PS);

	inline void setupMVP();

	inline D3D12_VIEWPORT getViewport(unsigned int width, unsigned int height) const
	{
		return D3D12_VIEWPORT{ 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
	};

	inline D3D12_RECT getScissorRect(unsigned int width, unsigned int height) const
	{
		return D3D12_RECT{ 0, 0, long(width), long(height) };
	}

};

