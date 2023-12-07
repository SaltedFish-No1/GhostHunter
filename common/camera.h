#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<tools.h>
#include <GLFW/glfw3.h>

#include<thread>

//global variables should be defined in *.cpp files instead of *.h files
//define gloabal variables in headers could arise redefination errors.
extern GLsizei window_width, window_height;
extern MouseInfo mouseInfo;

class Camera {
public:
	Timer timer;
	static const float gravity;
	static float moveSpeed;
	/*bool isPaused = true;*/

	Camera(GLFWwindow* window, glm::vec3 pos);
	Camera(GLFWwindow* window);

	float getFOV()
	{
		return FOV;
	}
	glm::mat4 getView()
	{
		return view;
	}
	glm::vec3 getPosition()
	{
		return viewPosition;
	}
	glm::vec3 getFront()
	{
		return front;;
	}
	glm::vec3 getRight()
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


