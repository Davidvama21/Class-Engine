#include "Globals.h"
#include "Application.h" 

#include "ModuleResources.h"

bool ModuleResources::init() {

    bool ok;

    ID3D12Device5* device = (app->getD3D12Module())->getDevice();

    // Command list and allocator initialization
    ok = SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)) );
    if (ok) {
        ok = SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
        if (ok)
            ok = SUCCEEDED(commandList->Close());
    }

    return ok;
}

bool ModuleResources::CreateUploadBuffer(const void* buffer, std::size_t numBytes, ComPtr<ID3D12Resource>& uploadBuffer, const LPCWSTR name)
{
    bool ok;

    // 1. Describe the buffer
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(numBytes);

    // 2. Specify UPLOAD heap properties
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    // 3. Create the resource
    ID3D12Device5* device = (app->getD3D12Module())->getDevice();
    ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)) );

    if (ok){
        uploadBuffer->SetName(name);

        // 4. Map the buffer: get a CPU pointer to its memory
        BYTE* pData = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
        ok = SUCCEEDED(uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)) );

        if (ok) {
            // 5. Copy our application data into the GPU buffer
            memcpy(pData, buffer, numBytes);

            // 6. Unmap the buffer (invalidate the pointer)
            uploadBuffer->Unmap(0, nullptr);
        }
    }

    return ok;
}

bool ModuleResources::CreateDefaultBuffer(const ComPtr<ID3D12Resource>& uploadBuffer, std::size_t numBytes, ComPtr<ID3D12Resource>& defaultBuffer, const LPCWSTR name)
{
    bool ok;

    D3D12Module* d3d12module = app->getD3D12Module();

    // 1. CREATE THE FINAL GPU BUFFER (DEFAULT HEAP)
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(numBytes);

    ID3D12Device5* device = d3d12module->getDevice();
    ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&defaultBuffer)) );

    if (ok) {
        defaultBuffer->SetName(name);

        // 2. SEND COPY COMMAND FROM UPLOAD TO DEFAULT

        ok = ok and SUCCEEDED(commandAllocator->Reset() ); // empty previous commands so that it doesn't fill up
        ok = ok and SUCCEEDED(commandList->Reset(commandAllocator.Get(), nullptr) );

        // Copy command (we do not need resource barriers here, since it is not a render operation)
        commandList->CopyResource(defaultBuffer.Get(), uploadBuffer.Get());

        ok = ok and SUCCEEDED(commandList->Close());
        if (ok) {

            ID3D12CommandList* listsToExecute[] = { commandList.Get() };
            ID3D12CommandQueue* queue = d3d12module->getCommandQueue();
            queue->ExecuteCommandLists(UINT(std::size(listsToExecute)), listsToExecute);

            // Now we wait so that the copy is finished (and thus it is ready if we need to use the data)
            d3d12module->flush();
        }
    }
    return ok;
}
