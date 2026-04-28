#pragma once
#include "tools.h"

struct GLFWwindow;

class App
{
public:
	GLFWwindow* window = nullptr;
	int windowWidth = 1600;
	int windowHeight = 1000;
	MouseInfo mouseInfo;

	void create();
	void shutdown();

	static App* fromWindow(GLFWwindow* w);

private:
	static void cursorPosThunk(GLFWwindow* w, double xpos, double ypos);
	static void framebufferSizeThunk(GLFWwindow* w, int width, int height);
};
