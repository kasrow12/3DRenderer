#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	float yaw;
	float pitch;
	float fov;

	const float cameraSpeed = 0.005f;
	const float sensitivity = 0.07f;
	const float upDownSpeed = 0.5f;

	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(position, position + front, up);
	}

	glm::mat4 getProjectionMatrix(float aspectRatio)
	{
		return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
	}

	void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += cameraSpeed * front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position -= cameraSpeed * front;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += glm::normalize(glm::cross(front, up)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			position += cameraSpeed * up * upDownSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			position -= cameraSpeed * up * upDownSpeed;
	}

	void processMouseMovement(float xOffset, float yOffset)
	{
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);
	}


};