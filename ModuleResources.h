#pragma once

#include "Module.h"
#include "D3D12Module.h"

class ModuleResources : public Module
{
public:

	bool init();

	bool CreateUploadBuffer(const void* buffer, std::size_t numBytes, ComPtr<ID3D12Resource>& uploadBuffer, const LPCWSTR name);

	// WARNING: numBytes has to be the exact same size of the upload buffer (TO CHANGE?)
	bool CreateDefaultBuffer(const ComPtr<ID3D12Resource>& uploadBuffer, std::size_t numBytes, ComPtr<ID3D12Resource>& defaultBuffer, const LPCWSTR name);

private:

	// Variables for default buffer copying to command
	ComPtr<ID3D12GraphicsCommandList4> commandList;
	ComPtr <ID3D12CommandAllocator> commandAllocator; // for the command list
};

