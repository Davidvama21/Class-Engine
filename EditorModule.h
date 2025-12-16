#pragma once

#include "Module.h"
#include "ImGuiPass.h"

#include "ModuleSampler.h"

class EditorModule : public Module
{
public:

	EditorModule(HWND hwnd);

	bool init();

	void update() override;

	void preRender() override;
	void render() override;
	void postRender() override;

	inline bool gridEnabled() const { return showGrid; };
	inline bool objectAxisEnabled() const { return showAxis; };
	inline int samplerType() const { return usedSampler; };

private:

	HWND hWnd;
	std::unique_ptr<ImGuiPass> imGUI = nullptr; // for GUI execution

	// Scene parameters
	bool showGrid = true;
	bool showAxis = true; // used for current selected object on screen
	int usedSampler = int (ModuleSampler::LINEAR_WRAP);

	void showExercise4Window();
};
