#pragma once

#include "Module.h"
#include "dxgi1_6.h" // para el factory

class D3D12Module : public Module
{
public:

    D3D12Module(HWND hwnd);
    ~D3D12Module();
    bool cleanUp() override;


    bool init();

    void preRender() override;
    void render() override;
    void postRender() override;

    inline D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetDescriptor() const {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameBuffIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
    };

    inline D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilDescriptor() const { // for now because there is a single descriptor
        return dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    };
    
    inline ID3D12Device5* getDevice() const { return device.Get(); };
    inline ID3D12GraphicsCommandList4* getCommandList() const { return commandList.Get(); };
    inline ID3D12CommandQueue* getCommandQueue() const { return queue.Get();  };
    inline ID3D12CommandAllocator* getCommandAllocator() const { return commandAllocators[currentFrameBuffIndex].Get(); };
    inline ID3D12Resource* getBackBuffer() const { return frameBuffers[currentFrameBuffIndex].Get(); };
    
    inline unsigned int getWindowWidth() const { return winWidth; };
    inline unsigned int getWindowHeight() const { return winHeight; };


    inline void flush(); // active wait for GPU things to finish


private:

    // Components for debugging

#if defined (_DEBUG)
    ComPtr<ID3D12Debug> debugInterface;
#endif

    // Components for graphics

    // Set up //
    ComPtr<ID3D12Device5> device;
    HWND hWnd = nullptr; // window handle

    // Commands //
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12GraphicsCommandList4> commandList; // for the queue

    ComPtr <ID3D12CommandAllocator> commandAllocators [FRAMES_IN_FLIGHT]; // so that we can handle each frame separately

    // Screen //
    unsigned int winWidth = 0;  // 0 means
    unsigned int winHeight = 0; // take window size at runtime

    ComPtr<IDXGIFactory6> factory;

    ComPtr<IDXGISwapChain4> swapChain;  // contains frame buffers
    ComPtr<ID3D12Resource> frameBuffers [FRAMES_IN_FLIGHT]; // (for quicker 
    unsigned int currentFrameBuffIndex;                 // access) <- this one has to be updated accordingly

    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // for passing frame buffers

    ComPtr<ID3D12Resource> depthStencilBuffer; // for now only so that we can add debug drawings (axis, grid)
    ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap; // (to reference it ^)

    // Synchronization //
    ComPtr<ID3D12Fence> queueFence; // will increase value when queue done executing
    UINT64 fenceValues [FRAMES_IN_FLIGHT] = {0}; // one per frame
    UINT64 currentExecution;
    
    HANDLE drawEvent = nullptr; // to associate fence values (and cause waiting until them)

    // Other options //
    bool supportsRT = false;
    bool allowTearing = false;


    void enableDebugLayer();
    bool createFactory();
    bool createDevice(bool useWarp);
    bool setupInfoQueue();
    bool createDrawCommandQueue();
    bool createSwapChain();
    bool createRenderTargets();
    bool createDepthStencil(); // for depth/stencil buffer
    bool createCommandList();
    bool createDrawFence();

    inline bool getWindowSize(unsigned int& width, unsigned int& height);

    inline void WaitForFence(UINT64 value);
};