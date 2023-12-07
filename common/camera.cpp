#include"camera.h"

const float Camera::gravity(9.8f);
float Camera::moveSpeed(5.0f);

Camera::Camera(GLFWwindow* window, glm::vec3 pos)
{
	viewPosition = pos;
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(front, up));
	view = glm::lookAt(viewPosition, viewPosition + front, up);
	this->window = window;
}


Camera::Camera(GLFWwindow* window)
{
	viewPosition = glm::vec3(0.0f, 0.0f, 5.0f); 
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(front, up));
	view = glm::lookAt(viewPosition, viewPosition + front, up);
	this->window = window;
	/*isPaused = false;*/
}

void Camera::goForward(float deltaTime)
{
	glm::vec3 direction = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
	viewPosition += direction * deltaTime * moveSpeed;
}

void Camera::goBackward(float deltaTime)
{
	glm::vec3 direction = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
	viewPosition -= direction * deltaTime * moveSpeed;
}

void Camera::goLeftward(float deltaTime)
{
	glm::vec3 direction = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
	viewPosition -= direction * deltaTime * moveSpeed;
}

void Camera::goRightward(float deltaTime)
{
	glm::vec3 direction = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
	viewPosition += direction * deltaTime * moveSpeed;
}

void Camera::defaultActions()
{
	timer.tictok();
	//update camera
	right = glm::normalize(glm::cross(front, up));
	view = glm::lookAt(viewPosition, viewPosition + front, up);
}

void Camera::processMouseMovement()
{
	//mouseInfo.yaw += mouseInfo.speed * mouseInfo.xoffset;
	//mouseInfo.pitch += mouseInfo.speed * mouseInfo.yoffset;
	float limitation = 50.0f;
	if (mouseInfo.pitch > limitation)
		mouseInfo.pitch = limitation;
 	if (mouseInfo.pitch < -limitation)
		mouseInfo.pitch = -limitation;

	front.x = cos(glm::radians(mouseInfo.yaw)) * cos(glm::radians(mouseInfo.pitch));
	front.y = sin(glm::radians(mouseInfo.pitch));
	front.z = sin(glm::radians(mouseInfo.yaw)) * cos(glm::radians(mouseInfo.pitch));
	front = glm::normalize(front);

}

void Camera::processKeyEvent()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		goLeftward(timer.getDeltaTime());
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		goRightward(timer.getDeltaTime());
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		goForward(timer.getDeltaTime());
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		goBackward(timer.getDeltaTime());
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) //pause
	{
		if (GameStatus::isPaused)
		{
			GameStatus::unpause(window);
		}
		else
		{
			GameStatus::pause(window);
		}
		while (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)//avoid quick lock-n-unclock
		{
			glfwPollEvents();
		}
	}
}
