#include "Globals.h"

#include "Application.h"
#include "D3D12Module.h"
#include "Mouse.h"
#include "Keyboard.h"

#include "ModuleCamera.h"

bool ModuleCamera::init()
{
    // Camera params.
    position = resetPosition = Vector3(0.0f, 10.0f, 10.0f);
    target = Vector3::Zero;
    up = Vector3::Up;

    d3d12Module = app->getD3D12Module();

    // Perspective params.
    aspectRatio = float(d3d12Module->getWindowWidth()) / float(d3d12Module->getWindowHeight());
    fov = XM_PIDIV4;
    zNear = 0.1f;
    zFar = 1000.0f;

    movementSpeed = 0.5f;

    view = Matrix::CreateLookAt(position, target, up);
    projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, zNear, zFar);

    lastWheelValue = Mouse::Get().GetState().scrollWheelValue;

    return true;
}

void ModuleCamera::update()
{
    // 1. Update projection
    aspectRatio = float(d3d12Module->getWindowWidth()) / float(d3d12Module->getWindowHeight());
    projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, zNear, zFar);

    // 2. Update view

    const Mouse::State& mouseState = Mouse::Get().GetState();
    const Keyboard::State& keyState = Keyboard::Get().GetState();

    Vector3 currentForward = XMVector3Normalize(target - position);
    Vector3 currentRight = XMVector3Normalize(XMVector3Cross(currentForward, up));

    float elapsedSec = app->getElapsedMilis() * 0.001f;
    float speed;
    if (keyState.LeftShift or keyState.RightShift) speed = 2 * movementSpeed;
    else speed = movementSpeed;

    // Actions //
    if (keyState.F) { // Reset position
        
        position = resetPosition;
    
    }else if ((keyState.LeftAlt or keyState.RightAlt) and mouseState.leftButton) // Orbit object
    {
        // ....
    }
    else if (mouseState.rightButton) { // Movement

        // Forward, backwards
        if (keyState.W)
            position += currentForward * speed * elapsedSec;
        else if (keyState.S)
            position -= currentForward * speed * elapsedSec;

        // Right, left
        if (keyState.D)
            position += currentRight * speed * elapsedSec;
        else if (keyState.A)
            position -= currentRight * speed * elapsedSec;

    }
    
    // Wheel zoom (we allow it always to happen; to change?)
    int change = mouseState.scrollWheelValue - lastWheelValue;
    if (change != 0)
        position += currentForward * float(change) * elapsedSec;

    lastWheelValue = mouseState.scrollWheelValue; // we update the value



    target = position + currentForward;
    view = Matrix::CreateLookAt(position, target, up);
}
