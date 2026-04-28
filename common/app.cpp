#include "app.h"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

App* App::fromWindow(GLFWwindow* w)
{
	return static_cast<App*>(glfwGetWindowUserPointer(w));
}

void App::cursorPosThunk(GLFWwindow* w, double xpos, double ypos)
{
	App* app = fromWindow(w);
	if (!app)
	{
		return;
	}
	MouseInfo& mi = app->mouseInfo;
	if (mi.firstMouse)
	{
		mi.lastX = (float)xpos;
		mi.lastY = (float)ypos;
		mi.firstMouse = false;
	}
	mi.comp_offset((float)xpos, (float)ypos);
	mi.reset_offset();
}

void App::framebufferSizeThunk(GLFWwindow* /*w*/, int width, int height)
{
	glViewport(0, 0, width, height);
}

void App::create()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Required for an OpenGL 3.3 Core context on macOS; harmless elsewhere.
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(windowWidth, windowHeight, "GhostHunter", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	mouseInfo.lastX = (float)windowWidth / 2.0f;
	mouseInfo.lastY = (float)windowHeight / 2.0f;

	glfwSetWindowUserPointer(window, this);
	glfwSetCursorPosCallback(window, &App::cursorPosThunk);
	glfwSetFramebufferSizeCallback(window, &App::framebufferSizeThunk);
	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void App::shutdown()
{
	glfwTerminate();
}
