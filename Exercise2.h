#pragma once

#include "Module.h"

class Exercise2 : public Module
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

	D3D12Module* d3d12module; // for easy access

	// Pipeline related objects //
	ComPtr<ID3D12Resource> vertexBuffer; // will contain vertex data on the GPU (is a default buffer)
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders

	ComPtr<ID3D12PipelineState> pipelineStateObject;

	inline bool uploadVertexData();
	inline bool createVertexSignature(ID3D12Device5* device);
	inline void addCompiledShaders(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);
	inline void setVShaderLayout (D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

	inline D3D12_VIEWPORT getViewport(unsigned int width, unsigned int height) const
	{
		return D3D12_VIEWPORT { 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
	};

	inline D3D12_RECT getScissorRect(unsigned int width, unsigned int height) const
	{
		return D3D12_RECT { 0, 0, long(width), long(height) };
	}

};

