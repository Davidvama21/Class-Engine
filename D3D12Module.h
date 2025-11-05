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


private:

    // Components for debugging

#if defined (_DEBUG)
    ComPtr<ID3D12Debug> debugInterface;
#endif

    // Components for graphics

    // Set up //
    ComPtr<ID3D12Device5> device;
    HWND hWnd; // window handle

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
    unsigned int currentFrameBuffIndex;                 // access) <- need to be updated accordingly

    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // for passing frame buffers

    // Synchronization //
    ComPtr<ID3D12Fence> queueFence; // will increase value when queue done executing
    UINT64 fenceValues [FRAMES_IN_FLIGHT] = {0}; // one per frame
    UINT64 currentExecution;
    
    HANDLE drawEvent; // to associate fence values (and cause waiting until them)

    // Other options //
    bool allowTearing = false;


    inline bool createDevice();

    inline void WaitForFence(UINT64 value);
    inline void waitForGPU(); // uses WaitForFence
};