#pragma once
#include<iostream>
#include<glad/glad.h>
#include <GLFW/glfw3.h> //GLFW should be include AFTER glad
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MouseInfo;
extern GLFWwindow* window;
extern GLsizei window_width, window_height;

struct Box
{
	glm::vec3 minVec; //(x_min, y_min, z_min)
	glm::vec3 maxVec; //(x_max, y_max, z_max)

};


/*-----------------------------------GLTools related-----------------------------------*/
class GLTools
{
public:
	static GLFWwindow* gltCreateContext();
	static unsigned int gltLoadTexture(char const* path);
};

/*-----------------------------------GameStatus related-----------------------------------*/

class Timer
{
private:
	float lastTime;
	float currentTime;
	float deltaTime;

public:
    Timer() :currentTime((float)glfwGetTime()), lastTime((float)glfwGetTime()), deltaTime(0.0f) {}
	
	/*
	Update currentTime, lastTime and deltaTime
	*/
	void tictok() 
	{
		lastTime = currentTime;
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
	}

	float getDeltaTime()
	{
		return deltaTime;
	}

	float getFPS()
	{
		return 1.0f / deltaTime;
	}

	void reset() 
	{
		lastTime = (float)glfwGetTime();
		currentTime = (float)glfwGetTime();
		deltaTime = 0.0;
	}
};

class MouseInfo {
public:
	float lastX = (float)window_width / 2.0f, lastY = (float)window_height / 2.0f;
	bool firstMouse = true;
	float xoffset = 0.0f; 
	float yoffset = 0.0f; 
	float yaw = 270.0f;   //Æ«º½½Ç
	float pitch = 0.0f; //¸©Ñö½Ç
	static float speed;

	void reset_offset();
	void comp_offset(float xpos, float ypos);
};

class GameStatus
{
public:
	static float groundHeight;
	static bool isPaused;

	static void pause(GLFWwindow* window);
	static void unpause(GLFWwindow* window);
};


/*-----------------------------------Callback functions-----------------------------------*/

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
