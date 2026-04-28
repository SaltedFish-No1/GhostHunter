#include "shaders.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace {

std::string readShaderSource(const char* path)
{
	std::ifstream stream(path, std::ios::in);
	if (!stream.is_open())
	{
		throw std::runtime_error(std::string("Failed to open shader file: ") + path);
	}
	std::stringstream sstr;
	sstr << stream.rdbuf();
	return sstr.str();
}

GLuint compileShader(GLenum stage, const char* source, const char* path)
{
	GLuint id = glCreateShader(stage);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	GLint status = GL_FALSE;
	GLint logLength = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
	if (status == GL_FALSE)
	{
		std::string log(logLength > 0 ? static_cast<size_t>(logLength) : 0, '\0');
		if (logLength > 0)
		{
			glGetShaderInfoLog(id, logLength, nullptr, log.data());
		}
		glDeleteShader(id);
		throw std::runtime_error(std::string("Failed to compile shader '") + path + "': " + log);
	}
	return id;
}

GLuint linkProgram(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint status = GL_FALSE;
	GLint logLength = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if (status == GL_FALSE)
	{
		std::string log(logLength > 0 ? static_cast<size_t>(logLength) : 0, '\0');
		if (logLength > 0)
		{
			glGetProgramInfoLog(program, logLength, nullptr, log.data());
		}
		glDetachShader(program, vs);
		glDetachShader(program, fs);
		glDeleteProgram(program);
		throw std::runtime_error(std::string("Failed to link shader program: ") + log);
	}

	glDetachShader(program, vs);
	glDetachShader(program, fs);
	return program;
}

} // namespace

Shader::Shader(const char* vertex_file_path, const char* fragment_file_path)
{
	const std::string vertexSource = readShaderSource(vertex_file_path);
	const std::string fragmentSource = readShaderSource(fragment_file_path);

	GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource.c_str(), vertex_file_path);
	GLuint fs = 0;
	try
	{
		fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str(), fragment_file_path);
	}
	catch (...)
	{
		glDeleteShader(vs);
		throw;
	}

	try
	{
		ID = linkProgram(vs, fs);
	}
	catch (...)
	{
		glDeleteShader(vs);
		glDeleteShader(fs);
		throw;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
