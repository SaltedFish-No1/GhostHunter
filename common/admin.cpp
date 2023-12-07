#include "admin.h"

void Admin::processInput()
{
	timer.tictok();
	processKeyEvent();     //keyboard
	processMouseMovement();//mouse movement
	defaultActions();
}
void Admin::fly(float deltaTime)
{
	viewPosition += glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime * moveSpeed;
}
void Admin::land(float deltaTime)
{
	viewPosition -= glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime * moveSpeed;
}
void Admin::processKeyEvent()
{
	Camera::processKeyEvent();
	//add fly and land function
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		fly(timer.getDeltaTime());
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		land(timer.getDeltaTime());
	}
}
