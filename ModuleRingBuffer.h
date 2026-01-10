#pragma once

#include "Module.h"

class ModuleRingBuffer : public Module
{
public:

	bool init();
	virtual void preRender() override;

	D3D12_GPU_VIRTUAL_ADDRESS allocBuffer(const void* buffer, size_t numBytes); // allocates data in the ring buffer, returns address to it (= 0 if error)

private:

	ComPtr<ID3D12Resource> ringBuffer;
	BYTE* pData; // CPU pointer to the buffer

	size_t totalSize, allocatedBytes;
	size_t head, tail; // indices of the buffer (head is first slot free, tail is slot AFTER the last free slot; tail can be 0, because buffer is circular!)

	size_t bytesAllocatedInFrame[FRAMES_IN_FLIGHT];

	unsigned int currentFrame = 0;

	// For easy access
	D3D12Module* d3d12Module;
};

