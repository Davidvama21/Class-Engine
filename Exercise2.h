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

	// Pipeline objects //
	ComPtr<ID3D12Resource> vertexBuffer; // will contain vertex data on the GPU (is a default buffer)
	ComPtr<ID3D12RootSignature> rootSignature; // param. specification for shaders
};

