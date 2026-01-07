#pragma once

#include "Module.h"

#include "Model.h"
#include "DebugDrawPass.h"

class Exercise5 : public Module
{
public:

	bool init();

	virtual void render() override;

private:

	Matrix mvp; // will contain transformations for vertices
	Matrix projection, view;

	std::string modelPath = "Assets/Models/Duck/Duck.gltf";
	std::string modelFolder = "Assets/Models/Duck/";
	float modelScale = 0.01f;

	std::unique_ptr <DebugDrawPass> debugDraw; // for grid, object arrows

	// For easy access
	D3D12Module* d3d12Module;
	EditorModule* editorModule;
	ModuleCamera* cameraModule;
	ModuleSampler* samplerModule;

	// Pipeline related objects //
	Model model; 
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders (to indicate passed paramateres)

	ComPtr<ID3D12PipelineState> pipelineStateObject;

	inline bool createVertexSignature(ID3D12Device5* device);
	inline bool createPipelineStateObject(ID3D12Device5* device);

	inline void getCompiledShaders(std::vector<uint8_t>& VS, std::vector<uint8_t>& PS);

	inline void setupMVP();

	D3D12_VIEWPORT getViewport(unsigned int width, unsigned int height) const
	{
		return D3D12_VIEWPORT{ 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
	};

	D3D12_RECT getScissorRect(unsigned int width, unsigned int height) const
	{
		return D3D12_RECT{ 0, 0, long(width), long(height) };
	}

};



