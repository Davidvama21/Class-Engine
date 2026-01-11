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
	//showExercise4Window();
	showExercise6Window();
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

void EditorModule::showExercise6Window()
{
	// Guizmo set up //
	ImGuizmo::BeginFrame(); // needed for guizmo functionality

	D3D12Module* d3d12 = app->getD3D12Module();
	unsigned width = d3d12->getWindowWidth();
	unsigned height = d3d12->getWindowHeight();

	// Set the viewport size (adjust based on your application)
	ImGuizmo::SetRect(0, 0, float(width), float(height));


	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Geometry Viewer Options");

	// Scene data
	ImGui::Text("FPS: %f", app->getFPS());
	ImGui::Checkbox("Show grid", &showGrid);
	ImGui::Checkbox("Show axis", &showAxis);
	ImGui::Checkbox("Show guizmo", &showGuizmo);
	
	ImGui::Separator();

	// Guizmo interface data
	if (ImGui::IsKeyPressed(ImGuiKey_T)) gizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) gizmoOperation = ImGuizmo::SCALE;

	ImGui::RadioButton("Translate", (int*)&gizmoOperation, (int)ImGuizmo::TRANSLATE);
	ImGui::SameLine();
	ImGui::RadioButton("Rotate", (int*)&gizmoOperation, ImGuizmo::ROTATE);
	ImGui::SameLine();
	ImGui::RadioButton("Scale", (int*)&gizmoOperation, ImGuizmo::SCALE);

	// Model data
	transformChanged = ImGui::DragFloat3("Translation", translation, 0.1f);
	transformChanged = transformChanged or ImGui::DragFloat3("Rotation", rotation, 0.1f);
	transformChanged = transformChanged or ImGui::DragFloat3("Scale", scale, 0.1f);


	// Light data
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Direction", lightDirection, 0.1f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Color", lightColor, ImGuiColorEditFlags_NoAlpha);
		ImGui::ColorEdit3("Ambient Color", ambientColor, ImGuiColorEditFlags_NoAlpha);
	}

	// Phong data
	if (ImGui::CollapsingHeader("Phong parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat("Kd", &Kd, 0.01f);
		ImGui::DragFloat("Ks", &Ks, 0.01f);
		ImGui::DragFloat("Shininess", &shininess, 0.01f);
	}

	ImGui::End();
}


