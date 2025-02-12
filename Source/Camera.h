#pragma once

// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
// with modifications to support different camera modes

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Transform.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

enum CameraMode {
    STATIC_SCENE,      // Static camera observing the scene
    STATIC_TRACKING,   // Static camera tracking moving object
    ATTACHED,          // Third person perspective
    FREE               // Free camera
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 7.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float SCROLL_SENSITIVITY = 2.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    CameraMode CurrentMode;
    Transform* TargetTransform;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW, float pitch = PITCH, CameraMode mode = FREE) : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), CurrentMode(mode)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix()
    {
        static glm::vec3 localOffset(11, 11, 0);
        static glm::vec3 viewDir(-1, -0.3, 0);

		switch (CurrentMode)
	    {
	    case STATIC_SCENE:
			return glm::lookAt(Position, glm::vec3(0.0f), Up);
		case STATIC_TRACKING:
			return glm::lookAt(Position, TargetTransform->position, WorldUp);
	    case ATTACHED:
	    	// third person perspective
            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(TargetTransform->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(TargetTransform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(TargetTransform->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::vec4 offset = rotationMatrix * glm::vec4(localOffset, 1.0f);
			glm::vec3 cameraPos = TargetTransform->position + glm::vec3(offset);
            glm::vec3 direction = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(viewDir, 0.0f)));
            Position = cameraPos;

			return glm::lookAt(Position, Position + direction, WorldUp);
	    case FREE:
	    default:
			return glm::lookAt(Position, Position + Front, Up);
	    }
    }

    void processKeyboard(Camera_Movement direction, float deltaTime)
    {
		if (CurrentMode != FREE) return;

        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
    }

	void setMode(CameraMode mode)
	{
		static glm::vec3 lastPosition = Position;
		if (CurrentMode == FREE)
			lastPosition = Position;

		CurrentMode = mode;
        if (mode == STATIC_SCENE)
			Position = glm::vec3(7.0f, 30.0f, 7.0f);
		if (mode == STATIC_TRACKING)
			Position = glm::vec3(0.0f, 17.0f, 0.0f);
		if (mode == FREE)
			Position = lastPosition;
	}

    void processMouseMovement(float xoffset, float yoffset)
    {
        if (CurrentMode != FREE) return;

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        updateCameraVectors();
    }

    void processMouseScroll(float yoffset)
    {
        Zoom -= yoffset * SCROLL_SENSITIVITY;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
