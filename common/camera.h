#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<tools.h>
#include <GLFW/glfw3.h>

#include<thread>

class Camera {
public:
	Timer timer;
	static const float gravity;
	static float moveSpeed;
	/*bool isPaused = true;*/

	Camera(GLFWwindow* window, glm::vec3 pos);
	Camera(GLFWwindow* window);

	float getFOV() const
	{
		return FOV;
	}
	glm::mat4 getView() const
	{
		return view;
	}
	glm::vec3 getPosition() const
	{
		return viewPosition;
	}
	glm::vec3 getFront() const
	{
		return front;
	}
	glm::vec3 getRight() const
	{
		return right;
	}
	void processKeyEvent();
	void processInput()
	{
		//timer.tictok(); //timer move into default actions
		processKeyEvent();     //keyboard
		processMouseMovement();//mouse movement
		defaultActions();
	}

protected:
	GLFWwindow* window;
	glm::vec3 viewPosition;
	glm::vec3 front;//direction rather a point
	glm::vec3 up;
	glm::vec3 right;
	glm::mat4 view;
	float FOV = 45.0f;


	void goForward(float deltaTime);
	void goBackward(float deltaTime);
	void goLeftward(float deltaTime);
	void goRightward(float deltaTime);
	void defaultActions();
	void processMouseMovement();
};


