// COMP710 GP Framework 2024

#include "shader.h"

#include "logmanager.h"
#include "matrix4.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
	std::string sShaderSearchRoot;
}

void Shader::setShaderSearchRoot(const std::string& rootDirectoryWithTrailingSlash)
{
	sShaderSearchRoot = rootDirectoryWithTrailingSlash;
}

Shader::Shader()
	: mVertexShader(0)
	, mPixelShader(0)
	, mShaderProgram(0)
{

}

Shader::~Shader()
{
	unload();
}

bool Shader::load(const char* vertexFile, const char* pixelFile)
{
	const std::string vertexPath =
		sShaderSearchRoot.empty() ? std::string(vertexFile) : (sShaderSearchRoot + vertexFile);
	const std::string pixelPath =
		sShaderSearchRoot.empty() ? std::string(pixelFile) : (sShaderSearchRoot + pixelFile);

	bool vertexCompiled = compileShader(vertexPath, GL_VERTEX_SHADER, mVertexShader);
	bool pixelCompiled = compileShader(pixelPath, GL_FRAGMENT_SHADER, mPixelShader);

	if (vertexCompiled == false || pixelCompiled == false)
	{
		LogManager::getInstance().log("Shaders failed to compile!");
		assert(0);
		return false;
	}

	mShaderProgram = glCreateProgram();

	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mPixelShader);
	glLinkProgram(mShaderProgram);

	return isValidProgram();
}

void Shader::unload()
{
	glDeleteProgram(mShaderProgram);
	glDeleteShader(mVertexShader);
	glDeleteShader(mPixelShader);
}

void Shader::setActive()
{
	assert(mShaderProgram);
	glUseProgram(mShaderProgram);
}

void Shader::setMatrixUniform(const char* name, const Matrix4& matrix)
{
	GLuint location = glGetUniformLocation(mShaderProgram, name);

	glUniformMatrix4fv(location, 1, GL_TRUE, (float*)&matrix);
}

void Shader::setVector4Uniform(const char* name, float x, float y, float z, float w)
{
	GLuint location = glGetUniformLocation(mShaderProgram, name);

	float vec4[4];

	vec4[0] = x;
	vec4[1] = y;
	vec4[2] = z;
	vec4[3] = w;

	glUniform4fv(location, 1, vec4);
}

bool Shader::compileShader(const std::string& fullPath, GLenum shaderType, GLuint& outShader)
{
	std::ifstream shaderFile(fullPath);

	if (shaderFile.is_open())
	{
		std::stringstream sstream;
		sstream << shaderFile.rdbuf();
		std::string shaderCode = sstream.str();
		const char* pShaderCode = shaderCode.c_str();

		outShader = glCreateShader(shaderType);
		glShaderSource(outShader, 1, &(pShaderCode), 0);
		glCompileShader(outShader);

		if (!isCompiled(outShader))
		{
			LogManager::getInstance().log("Shader failed to compile!");
			return false;
		}
	}
	else
	{
		LogManager::getInstance().log("Shader file not found!");
		return false;
	}

	return true;
}

bool Shader::isCompiled(GLuint shader)
{
	GLint compiledStatus;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiledStatus);

	if (compiledStatus != GL_TRUE)
	{
		char error[1024];
		error[0] = 0;
		glGetShaderInfoLog(shader, 1023, 0, error);

		LogManager::getInstance().log("Shader failed to compile!");

		return false;
	}

	return true;
}

bool Shader::isValidProgram()
{
	GLint linkedStatus;

	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &linkedStatus);

	if (linkedStatus != GL_TRUE)
	{
		char error[1024];
		error[0] = 0;
		glGetShaderInfoLog(mShaderProgram, 1023, 0, error);

		LogManager::getInstance().log("Shader failed to link!");
		assert(0);

		return false;
	}

	return true;
}
