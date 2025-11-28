#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "EditorModule.h" 

EditorModule::EditorModule(HWND hwnd): hWnd(hwnd)
{
}


bool EditorModule::init()
{
	ID3D12Device5* device = (app->getD3D12Module())->getDevice();

	imGUI = std::unique_ptr<ImGuiPass> (new ImGuiPass(device, hWnd)); // cpu, gpu handles could be indicated, so that we don't use default ones

	return true;
}

void EditorModule::update()
{
}

void EditorModule::preRender()
{
	imGUI->startFrame();

	// Interface calls (== interface elements) //
}

void EditorModule::render()
{

	//imGUI->record( (app->getD3D12Module())->getCommandList() );
}

void EditorModule::postRender()
{
	imGUI->record((app->getD3D12Module())->getCommandList());
}


