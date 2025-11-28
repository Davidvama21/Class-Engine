#pragma once

#include "Module.h"
#include "ImGuiPass.h"

class EditorModule : public Module
{
public:

	EditorModule(HWND hwnd);

	bool init();

	void update() override;

	void preRender() override;
	void render() override;
	void postRender() override;

private:

	HWND hWnd;
	std::unique_ptr<ImGuiPass> imGUI = nullptr; // for GUI execution
};
