// COMP710 GP Framework 2024
#ifndef __SHADER_H_
#define __SHADER_H_

#include <GL/glew.h>

#include <string>

struct Matrix4;

class Shader
{
public:
	Shader();
	~Shader();

	static void setShaderSearchRoot(const std::string& rootDirectoryWithTrailingSlash);

	bool load(const char* vertexFile, const char* pixelFile);
	void unload();

	void setActive();

	void setMatrixUniform(const char* name, const Matrix4& matrix);
	void setVector4Uniform(const char* name, float x, float y, float z, float w);
	void setFloatUniform(const char* name, float x);
	void setIntUniform(const char* name, int value);
	void setVec4ArrayUniform(const char* name, const float* vec4s, int count);

private:
	bool isValidProgram();

	static bool compileShader(const std::string& fullPath, GLenum shaderType, GLuint& outShader);
	static bool isCompiled(GLuint shader);

	GLuint mVertexShader;
	GLuint mPixelShader;
	GLuint mShaderProgram;
};

#endif // __SHADER_H_
