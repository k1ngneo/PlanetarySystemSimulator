#pragma once

#include <string>
#include <GLAD/glad.h>

#include <glm/glm.hpp>

#include <string>
#include <filesystem>

const int VERTEX_SHADER = 1;
const int FRAGMENT_SHADER = 2;

class Shader
{
	uint32_t m_Program;
	uint32_t m_VertexShaderID;
	uint32_t m_FragmentShaderID;

	std::string m_Path;
	std::filesystem::file_time_type m_LastModTime;

public:
	//next attribute number to assign
	static unsigned int numAttributes;

	// number of the first attribute in a shader
	unsigned int m_AttributeNum;

private:
	void compileShader(uint32_t shader, const char* filePath);

public:
	Shader();
	~Shader();
	void compileShaders(const char* source, bool path = false);
	void compileShaders(const char* vertexPath, const char* fragmentPath);
	void linkShaders();

	void reload();

	void use();
	void unuse();

	void addAttribute(const char* attributeName);

	int32_t getUniformLocation(const char* uniformName);

	void setUniform1f(const char* name, float value);
	void setUniform2f(const char* name, glm::vec2 vector);
	void setUniform3f(const char* name, glm::vec3 vector);
	void setUniform4f(const char* name, glm::vec4 vector);
	void setUniform1i(const char* name, int value);
	void setUniform1ui(const char* name, unsigned int value);
	void setUniformMat4(const char* name, const glm::mat4& matrix);

	inline uint32_t GetProgram() { return m_Program; }
};