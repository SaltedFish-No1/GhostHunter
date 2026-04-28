#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h> //GLFW should be include AFTER glad
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MouseInfo;

struct Box
{
	glm::vec3 minVec; //(x_min, y_min, z_min)
	glm::vec3 maxVec; //(x_max, y_max, z_max)
};


/*-----------------------------------GLTools related-----------------------------------*/
class GLTools
{
public:
	static unsigned int gltLoadTexture(char const* path);

	// Resolve the directory containing the running executable on Windows /
	// macOS / Linux, then chdir into it. After this call, the legacy
	// "../res/..." relative paths resolve against the staged resource
	// layout (build/bin/res/) regardless of where the user invoked the
	// binary from. Safe to call once at startup.
	static void anchorWorkingDirectoryToExecutable();
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
	float lastX = 0.0f;
	float lastY = 0.0f;
	bool firstMouse = true;
	float xoffset = 0.0f;
	float yoffset = 0.0f;
	float yaw = 270.0f;
	float pitch = 0.0f;
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
