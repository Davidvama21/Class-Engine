#include "Globals.h"

#include "Application.h"
#include "D3D12Module.h"
#include "Keyboard.h"

#include "ModuleCamera.h"

bool ModuleCamera::init()
{
    // Camera params.
    position = Vector3(0.0f, 10.0f, 10.0f);
    target = objectPosition = Vector3::Zero;
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

    const Mouse::State& mouseState = Mouse::Get().GetState();
    lastWheelValue = mouseState.scrollWheelValue;
    lastXMousePos = mouseState.x;
    lastYMousePos = mouseState.y;

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
    if (keyState.F) { // Reset to object's position
        
        position = objectPosition - currentForward*10*sqrt(2); // 10*sqrt(2) = sqrt(200) is the module of the distance to the object at the beginning (on init())
    
    }else if ((keyState.LeftAlt or keyState.RightAlt) and mouseState.leftButton) // Orbit object
    {
        int deltaMouseX, deltaMouseY;
        getMouseDeltas(deltaMouseX, deltaMouseY, mouseState);

        if (deltaMouseX != 0 or deltaMouseY != 0) {
        
            // 1. Obtain new camera position (altering position-objectPosition vector)
            Vector3 rotation;
            rotation.x = -deltaMouseY * speed * elapsedSec; // negative, because we want to rotate in the opposite direction (this is the pitch)
            rotation.z = 0;
            rotation.y = -deltaMouseX * speed * elapsedSec;

            Quaternion rotationQuat = Quaternion::CreateFromYawPitchRoll(rotation);
            Vector3 objectToCam = position - objectPosition;
            objectToCam = Vector3::Transform(objectToCam, rotationQuat); // now it leads to the new camera position

            position = objectPosition + objectToCam;
            
            // 2. Update current forward
            currentForward = XMVector3Normalize(-objectToCam);

            /*
            // 3. Calculate new up, based on ideal visualization 
            Quaternion upwardRot(XMConvertToRadians(-90), 0, 0, 1);
            up = Vector3::Transform(up, upwardRot);
            */
        }
       
    }
    else if (mouseState.rightButton) { // Movement

        // Look around (mouse)
        int deltaMouseX, deltaMouseY;
        getMouseDeltas(deltaMouseX, deltaMouseY, mouseState);

        if (deltaMouseX != 0 or deltaMouseY != 0) {

            Vector3 rotation;
            rotation.x = -deltaMouseY * speed * elapsedSec; // negative, because we want to rotate in the opposite direction (this is the pitch)
            rotation.z = 0; 
            rotation.y = -deltaMouseX * speed * elapsedSec;
            
            Quaternion rotationQuat = Quaternion::CreateFromYawPitchRoll(rotation);

            currentForward = Vector3::Transform(currentForward, rotationQuat);
            //currentForward = XMVector3Normalize(currentForward);

            //up = Vector3::Transform(up, rotationQuat); // TO REVISE
            //up = XMVector3Normalize(up);

            // (Before continuing, we should ensure that up is not too tilted)

            currentRight = XMVector3Normalize(XMVector3Cross(currentForward, up)); // recalculate because it has changed
        }

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

        // Up, down
        if (keyState.Q)
            position += up * speed * elapsedSec;
        else if (keyState.E)
            position -= up * speed * elapsedSec;
    }
    
    // Wheel zoom (we allow it always to happen; to change?)
    int change = mouseState.scrollWheelValue - lastWheelValue;
    if (change != 0)
        position += currentForward * float(change) * elapsedSec;

    // We update saved mouse values
    lastWheelValue = mouseState.scrollWheelValue;
    lastXMousePos = mouseState.x;
    lastYMousePos = mouseState.y;


    target = position + currentForward;
    view = Matrix::CreateLookAt(position, target, up);
}

inline void ModuleCamera::getMouseDeltas(int& xPos, int& yPos, const Mouse::State& mouseState) {

    // For now, we will go for the simplest implementation
    xPos = mouseState.x - lastXMousePos;
    yPos = mouseState.y - lastYMousePos;
}
