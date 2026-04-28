#include "tools.h"

#include "app.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //load texture
#endif // !STB_IMAGE_IMPLEMENTATION

#include <filesystem>
#include <system_error>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <climits>
#elif defined(__linux__)
    #include <unistd.h>
    #include <climits>
#endif

float MouseInfo::speed(0.05f);


/*-----------------------------------GLTools related-----------------------------------*/
namespace {
std::filesystem::path executablePath()
{
#if defined(_WIN32)
	wchar_t buf[MAX_PATH];
	DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
	if (n == 0 || n == MAX_PATH) return {};
	return std::filesystem::path(buf, buf + n);
#elif defined(__APPLE__)
	char buf[PATH_MAX];
	uint32_t size = sizeof(buf);
	if (_NSGetExecutablePath(buf, &size) != 0) return {};
	return std::filesystem::path(buf);
#elif defined(__linux__)
	char buf[PATH_MAX];
	ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf));
	if (n <= 0) return {};
	return std::filesystem::path(std::string(buf, static_cast<size_t>(n)));
#else
	return {};
#endif
}
} // namespace

void GLTools::anchorWorkingDirectoryToExecutable()
{
	std::filesystem::path exe = executablePath();
	if (exe.empty()) return;
	std::error_code ec;
	std::filesystem::path dir = std::filesystem::canonical(exe, ec).parent_path();
	if (ec || dir.empty()) return;
	std::filesystem::current_path(dir, ec);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	isPaused = true;
	if (App* app = App::fromWindow(window))
	{
		app->mouseInfo.firstMouse = true;
	}
}

void GameStatus::unpause(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	isPaused = false;
}

/*-----------------------------------MouseInfo related-----------------------------------*/
void MouseInfo::comp_offset(float xpos, float ypos)
{
	if (!GameStatus::isPaused)
	{
		xoffset = xpos - lastX;
		yoffset = lastY - ypos;

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
