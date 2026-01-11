#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"

#include "ModuleRingBuffer.h"

bool ModuleRingBuffer::init() {

    // 1. Describe the buffer

    totalSize = alignUp(RING_BUFFER_SIZE, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT); // we save it, for future checks
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

    // 2. Specify UPLOAD heap properties
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    // 3. Create the resource
    d3d12Module = app->getD3D12Module();
    ID3D12Device5* device = d3d12Module->getDevice();
    bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&ringBuffer)));

    if (ok) {
        ringBuffer->SetName(L"Default ring buffer"); // so that we can identify it when debugging

        // 4. Map the buffer: get a CPU pointer to its memory
        pData = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
        ok = SUCCEEDED(ringBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)));

        if (ok) {
            // 5. Set up the rest of the parameters

            head = tail = allocatedBytes = 0;

            for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; ++i)
                bytesAllocatedInFrame[i] = 0;
        }
    }
    return ok;
}

void ModuleRingBuffer::preRender()
{
    currentFrame = d3d12Module->getBackBufferIndex();

    // Free data (should be safe, since current frame previous execution is finished)
    tail = (tail + bytesAllocatedInFrame[currentFrame]) % totalSize; // this frees the oldest reserved data, if we turned around the buffer
    allocatedBytes -= bytesAllocatedInFrame[currentFrame];

    bytesAllocatedInFrame[currentFrame] = 0;
}

D3D12_GPU_VIRTUAL_ADDRESS ModuleRingBuffer::allocBuffer(const void* buffer, size_t numBytes)
{
	numBytes = alignUp(numBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT); // to keep data aligned
	
	if (tail > head) {

		if (numBytes <= tail - head) { // there's space to allocate the data

			memcpy(pData + head, buffer, numBytes);

			D3D12_GPU_VIRTUAL_ADDRESS address = ringBuffer->GetGPUVirtualAddress() + head;
			head = head + numBytes; // update head

			bytesAllocatedInFrame[currentFrame] += numBytes; // count used bytes for future deallocations
			allocatedBytes += numBytes;

			return address;
		}
		
		return D3D12_GPU_VIRTUAL_ADDRESS(0);


	}else if (tail < head) {

		if (numBytes <= totalSize - head) { // there's space to allocate (at THE END, we don't save data circularly)

			memcpy(pData + head, buffer, numBytes);

			D3D12_GPU_VIRTUAL_ADDRESS address = ringBuffer->GetGPUVirtualAddress() + head;
			head = (head + numBytes) % totalSize; // update head (with module, in case it ends at the beginning of the buffer)

			bytesAllocatedInFrame[currentFrame] += numBytes; // count used bytes for future deallocations
			allocatedBytes += numBytes;

			return address;

		}else{
			// Retry from the beginning
			bytesAllocatedInFrame[currentFrame] += totalSize - head - 1; // count unused bytes from the end, for future deallocations
			allocatedBytes += totalSize - head - 1;

			head = 0;
			return allocBuffer(buffer, numBytes);
		}


	}else{ // tail == head

		if (allocatedBytes != totalSize) { // => all memory is free 

			if (numBytes <= totalSize) {
				// Allocate from 0

				memcpy(pData, buffer, numBytes);

				D3D12_GPU_VIRTUAL_ADDRESS address = ringBuffer->GetGPUVirtualAddress() + head;
				head = (head + numBytes) % totalSize;

				bytesAllocatedInFrame[currentFrame] += numBytes; // count used bytes for future deallocations

				return address;
			}
		}
		return D3D12_GPU_VIRTUAL_ADDRESS(0); // either full or numBytes bigger than the ring buffer
	}
}
