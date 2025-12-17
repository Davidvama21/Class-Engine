#pragma once

#include "Module.h"

class D3D12Module;

class ModuleCamera : public Module
{
public:

	bool init() override;

	void update() override;

	inline Matrix getProjectionMatrix() const { return projection; };
	inline Matrix getViewMatrix() const { return view; };


private:

	// View parameters
	Vector3 position, target, up; 

	// Projection parameters
	float fov, aspectRatio;
	float zNear, zFar;

	Matrix view, projection; // for easy access (updated in update())

	float movementSpeed; // for key, mouse movement
	Vector3 resetPosition; // (when pressing 'F')
	int lastWheelValue;

	D3D12Module* d3d12Module;
};

