#pragma once

#include "Module.h"

#include "Model.h"
#include "DebugDrawPass.h"

struct PerInstance { // frame data that changes per mesh

	Matrix modelMat;
	Matrix normalMat;

	PhongMaterialData material;
};

struct PerFrame // general frame data (padding is to keep shader cbuffer alignment)
{
	Vector3 L = Vector3::UnitX; // light direction
	float pad0;
	Vector3 Lc = Vector3::One; // light color
	float pad1;
	Vector3 Ac = Vector3::Zero; // ambient color
	float pad2;
	Vector3 viewPos = Vector3::Zero; // camera position
	float pad3;
};



class Exercise6 : public Module
{
public:

	bool init();

	virtual void render() override;

private:

	Matrix mvp; // will contain transformations for vertices
	Matrix projection, view;

	Matrix modelNoScale; // model matrix for the debug drawings

	PerFrame lightingData;

	std::string modelPath = "Assets/Models/Duck/Duck.gltf";
	std::string modelFolder = "Assets/Models/Duck/";
	float modelScale = 0.01f;

	std::unique_ptr <DebugDrawPass> debugDraw; // for grid, object arrows

	// For easy access
	D3D12Module* d3d12Module;
	EditorModule* editorModule;
	ModuleCamera* cameraModule;
	ModuleSampler* samplerModule;
	ModuleRingBuffer* ringBufferModule;

	// Pipeline related objects //
	Model model;
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders (to indicate passed paramateres)

	ComPtr<ID3D12PipelineState> pipelineStateObject;

	inline bool loadModelData();

	inline bool createVertexSignature(ID3D12Device5* device);
	inline bool createPipelineStateObject(ID3D12Device5* device);

	inline void getCompiledShaders(std::vector<uint8_t>& VS, std::vector<uint8_t>& PS);

	inline void setupMVP();

	inline void setupLighting();

	inline void updateModelDataInEditor(float scale[], float rotation[], float translation[], ImGuizmo::OPERATION guizmoOp);

	D3D12_VIEWPORT getViewport(unsigned int width, unsigned int height) const
	{
		return D3D12_VIEWPORT{ 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
	};

	D3D12_RECT getScissorRect(unsigned int width, unsigned int height) const
	{
		return D3D12_RECT{ 0, 0, long(width), long(height) };
	};

};

