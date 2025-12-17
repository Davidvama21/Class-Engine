#pragma once

#include "Globals.h"

#include <array>
#include <vector>
#include <chrono>

class Module;
class D3D12Module;
class EditorModule;
class ModuleResources;
class ModuleCamera;
class ModuleShaderDescriptors;
class ModuleSampler;

class Application
{
public:

	Application(int argc, wchar_t** argv, void* hWnd);
	~Application();

	bool         init();
	void         update();
	bool         cleanUp();

    
    float                       getFPS() const { return 1000.0f * float(MAX_FPS_TICKS) / tickSum; }
    float                       getAvgElapsedMs() const { return tickSum / float(MAX_FPS_TICKS); }
    uint64_t                    getElapsedMilis() const { return elapsedMilis; }

    bool                        isPaused() const { return paused; }
    bool                        setPaused(bool p) { paused = p; return paused; }

    inline D3D12Module* getD3D12Module() const { return d3d12Module; };
    inline EditorModule* getEditorModule() const { return editorModule; };
    inline ModuleResources* getModuleResources() const { return resourcesModule; };
    inline ModuleCamera* getModuleCamera() const { return cameraModule; };
    inline ModuleShaderDescriptors* getModuleShaderDesc() const { return shaderDescModule; };
    inline ModuleSampler* getModuleSampler() const { return samplerModule; };

private:
    enum { MAX_FPS_TICKS = 30 };
    typedef std::array<uint64_t, MAX_FPS_TICKS> TickList;

    std::vector<Module*> modules;
	
    D3D12Module* d3d12Module;
    EditorModule* editorModule;
    ModuleResources* resourcesModule;
    ModuleCamera* cameraModule;
    ModuleShaderDescriptors* shaderDescModule;
    ModuleSampler* samplerModule;

    uint64_t  lastMilis = 0;
    TickList  tickList;
    uint64_t  tickIndex;
    uint64_t  tickSum = 0;
    uint64_t  elapsedMilis = 0;
    bool      paused = false;
};

extern Application* app;
