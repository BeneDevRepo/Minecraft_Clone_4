#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
	UP,
	DOWN
};

const float YAW         = -90.0f;
const float PITCH       =  0.0f;
// const float SPEED       =  3.5f;
const float SPEED       =  30.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;

    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
    inline glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

private:
	void updateCameraVectors();
};



#include "Window/winapi_include.h"

inline void updateCamera(Camera &camera, const int mouseX, const int mouseY, const float dt) {
	if (GetAsyncKeyState('W') & 0x8000)
		camera.ProcessKeyboard(FORWARD, dt);
	if (GetAsyncKeyState('S') & 0x8000)
		camera.ProcessKeyboard(BACKWARD, dt);
	if (GetAsyncKeyState('A') & 0x8000)
		camera.ProcessKeyboard(LEFT, dt);
	if (GetAsyncKeyState('D') & 0x8000)
		camera.ProcessKeyboard(RIGHT, dt);
	if (GetAsyncKeyState(' ') & 0x8000)
		camera.ProcessKeyboard(UP, dt);
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		camera.ProcessKeyboard(DOWN, dt);

	static int pmouseX = mouseX, pmouseY = mouseY;
	
	if(GetKeyState(VK_LBUTTON) & 0x8000)
		camera.ProcessMouseMovement(-(mouseX - pmouseX), (mouseY - pmouseY));
		// camera.ProcessMouseMovement(mouseX - pmouseX, -(mouseY - pmouseY));

	pmouseX = mouseX;
	pmouseY = mouseY;
}