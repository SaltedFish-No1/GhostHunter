#include"tools.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //load texture
#endif // !STB_IMAGE_IMPLEMENTATION

float MouseInfo::speed(0.05f);
MouseInfo mouseInfo;


/*-----------------------------------GLTools related-----------------------------------*/
GLFWwindow* GLTools::gltCreateContext()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}
	return window;
}

unsigned int GLTools::gltLoadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

/*-----------------------------------GameStatus related-----------------------------------*/
bool GameStatus::isPaused(false);

void GameStatus::pause(GLFWwindow* window)
{
	//unlock the cursor into the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //hidden and disabled the cursor
	isPaused = true;
	mouseInfo.firstMouse = true;
}

void GameStatus::unpause(GLFWwindow* window)
{
	//Lock the cursor into the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //hidden and disabled the cursor
	isPaused = false;
}

/*-----------------------------------MouseInfo related-----------------------------------*/
void MouseInfo::comp_offset(float xpos, float ypos)
{
	if (!GameStatus::isPaused)//if paused, do not update camera
	{
		xoffset = xpos - lastX;
		yoffset = lastY - ypos; //curse up, view down

		yaw += speed * xoffset;
		pitch += speed * yoffset;
	}
	lastX = xpos;
	lastY = ypos;
}

void MouseInfo::reset_offset()
{
	xoffset = 0.0f;
	yoffset = 0.0f;
}

/*-----------------------------------Callback functions-----------------------------------*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouseInfo.firstMouse)
	{
		mouseInfo.lastX = (float)xpos;
		mouseInfo.lastY = (float)ypos;
		mouseInfo.firstMouse = false;
	}
	mouseInfo.comp_offset((float)xpos, (float)ypos);
	mouseInfo.reset_offset();

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
