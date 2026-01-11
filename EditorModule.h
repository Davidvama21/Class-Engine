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

	bool gridEnabled() const { return showGrid; };
	bool objectAxisEnabled() const { return showAxis; };
	int samplerType() const { return usedSampler; };

	bool changedTransform() const { return transformChanged; };

	Vector3 getScale() const { return Vector3(scale[0], scale[1], scale[2]); };
	Vector3 getRotation() const { return Vector3(rotation[0], rotation[1], rotation[2]); };
	Vector3 getTranslation() const { return Vector3(translation[0], translation[1], translation[2]); };

	Vector3 getLightDirection() const { return Vector3(lightDirection[0], lightDirection[1], lightDirection[2]); };
	Vector3 getLightColor() const { return Vector3(lightColor[0], lightColor[1], lightColor[2]); };
	Vector3 getAmbientColor() const { return Vector3(ambientColor[0], ambientColor[1], ambientColor[2]); };

	float getPhongKd() const { return Kd; };
	float getPhongKs() const { return Ks; };
	float getPhongShininess() const { return shininess; };

private:

	HWND hWnd;
	std::unique_ptr<ImGuiPass> imGUI = nullptr; // for GUI execution

	// Scene parameters
	bool showGrid = true;
	bool showAxis = true; // used for current selected object on screen
	int usedSampler = int (ModuleSampler::LINEAR_WRAP);

	// Model parameters
	float scale[3] = {1.f, 1.f, 1.f}; // we use arrays because that is the required format for Dear imGui calls
	float rotation[3] = { 0.f, 0.f, 0.f };
	float translation[3] = { 0.f, 0.f, 0.f };

	bool transformChanged = false;

	// Light parameters
	float lightDirection[3] = { -0.5f, -0.5f, -0.5f };
	float lightColor[3] = { 1.f, 1.f, 1.f };
	float ambientColor[3] = { 0.1f, 0.1f, 0.1f };

	// Phong parameters
	float Kd = 0.85f;
	float Ks = 0.35f;
	float shininess = 32.0f;

	void showExercise4Window();
	void showExercise6Window();
};
