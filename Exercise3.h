#pragma once

#include "Module.h"

#include "DebugDrawPass.h"

class Exercise3 : public Module
{
public:

	bool init();

	virtual void render() override;

private:

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[3] =
	{
		{-1.0f, -1.0f, 0.0f }, // 0
		{ 0.0f, 1.0f, 0.0f }, // 1
		{ 1.0f, -1.0f, 0.0f } // 2
	};

	Matrix mvp; // will contain transformations for vertices
	Matrix projection, view;

	std::unique_ptr <DebugDrawPass> debugDraw; // for grid, object arrows

	D3D12Module* d3d12module; // for easy access

	// Pipeline related objects //
	ComPtr<ID3D12Resource> vertexBuffer; // will contain vertex data on the GPU (is a default buffer)
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders (to indicate passed paramateres)

	ComPtr<ID3D12PipelineState> pipelineStateObject;

	inline bool uploadVertexData();
	inline bool createVertexSignature(ID3D12Device5* device);
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


