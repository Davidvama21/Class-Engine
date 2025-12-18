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
	showExercise4Window();
}

void EditorModule::render()
{

}

void EditorModule::postRender()
{
	imGUI->record((app->getD3D12Module())->getCommandList());
}

void EditorModule::showExercise4Window()
{
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Texture Viewer Options");

	ImGui::Text("FPS: %f", app->getFPS());
	ImGui::Checkbox("Show grid", &showGrid);
	ImGui::Checkbox("Show axis", &showAxis);
	ImGui::Combo("Sampler", &usedSampler, "Linear/Wrap\0Point/Wrap\0Linear/Clamp\0Point/Clamp\0", ModuleSampler::MAX_SAMPLERS);

	ImGui::End();
}


