#pragma once

#include "Module.h"

#include "Mouse.h"

class D3D12Module;

class ModuleCamera : public Module
{
public:

	bool init() override;

	void update() override;

	Matrix getProjectionMatrix() const { return projection; };
	Matrix getViewMatrix() const { return view; };

	Vector3 getPosition() const { return position; };

private:

	// View parameters
	Vector3 position, target, up; 

	// Projection parameters
	float fov, aspectRatio; // fov is horizontal fov
	float zNear, zFar;

	Matrix view, projection; // for easy access (updated in update())

	float movementSpeed; // for key, mouse movement
	Vector3 objectPosition; // where we look at (for resetting position when pressing 'F')
	int lastWheelValue;
	int lastXMousePos, lastYMousePos;

	D3D12Module* d3d12Module;

	inline void getMouseDeltas(int& xPos, int& yPos, const Mouse::State& mouseState);
};

