#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/utilities/error.h"
#include "StarSystemSim/utilities/load_text_file.h"
#include "StarSystemSim/utilities/string_manipulations.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <cstring>


unsigned int Shader::numAttributes = 0;

Shader::Shader()
	: m_FragmentShaderID(0), m_VertexShaderID(0), m_GeometryShaderID(0), m_Program(0), m_AttributeNum(0)
{
}

Shader::~Shader() {
	if (m_Program != 0)
		glDeleteProgram(m_Program);
}

void Shader::use() {
	glUseProgram(m_Program);

	for (unsigned int i = 0; i < numAttributes; i++) {
		glEnableVertexAttribArray(i);
	}
}
void Shader::unuse() {
	glUseProgram(0);

	for (unsigned int i = 0; i < numAttributes; i++) {
		glDisableVertexAttribArray(i);
	}
}

void Shader::addAttribute(const char* attributeName) {
	glBindAttribLocation(m_Program, numAttributes++, attributeName);
}

GLint Shader::getUniformLocation(const char* uniformName) {
	return glGetUniformLocation(m_Program, uniformName);
}

void Shader::setUniform1f(const char* name, float value) {
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform2f(const char* name, glm::vec2 vector) {
	glUniform2f(getUniformLocation(name), vector.x, vector.y);
}

void Shader::setUniform3f(const char* name, glm::vec3 vector) {
	glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z);
}

void Shader::setUniform4f(const char* name, glm::vec4 vector) {
	glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
}

void Shader::setUniform1i(const char* name, int value) {
	glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform1ui(const char* name, unsigned int value) {
	glUniform1ui(getUniformLocation(name), value);
}

void Shader::setUniformMat4(const char* name, const glm::mat4& matrix) {
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::compileShaders(const char* src, bool path) {
	std::string source = src;

	if (path) {
		m_Path = src;
		source = utils::loadTextFile(src);
		m_LastModTime = std::filesystem::last_write_time(src);
	}

	m_Program = glCreateProgram();

	m_VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	m_FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexSource = "";
	std::string fragmentSource = "";
	std::string geometrySource = "";
	std::string line;

	enum class ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER, NO_SHADER } shader_type;
	shader_type = ShaderType::NO_SHADER;

	const std::string vertexShaderPrep = "#vertex_shader";
	const std::string fragmentShaderPrep = "#fragment_shader";
	const std::string geometryShaderPrep = "#geometry_shader";

	for (unsigned int i = 0; i < utils::howManyLines(source.c_str()); ++i) {
		line = utils::getLine(source.c_str(), i);

		bool isItPrepCom = false;

		if (!line.compare(vertexShaderPrep)) {
			shader_type = ShaderType::VERTEX_SHADER;
			isItPrepCom = true;
		}
		else if (!line.compare(fragmentShaderPrep)) {
			shader_type = ShaderType::FRAGMENT_SHADER;
			isItPrepCom = true;
		}
		else if (!line.compare(geometryShaderPrep)) {
			shader_type = ShaderType::GEOMETRY_SHADER;
			isItPrepCom = true;

			m_GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		}

		if (!isItPrepCom) {
			switch (shader_type) {
				case ShaderType::VERTEX_SHADER:
				{
					vertexSource += line + "\n";
				} break;
				case ShaderType::FRAGMENT_SHADER:
				{
					fragmentSource += line + "\n";
				} break;
				case ShaderType::GEOMETRY_SHADER:
				{
					geometrySource += line + "\n";
				} break;
				case ShaderType::NO_SHADER:
				{
				} break;
			}
		}
	}

	//printf( "VERTEX_SHADER:\n%s", vertexSource.c_str() );
	//printf( "GEOMETRY_SHADER:\n%s", geometrySource.c_str() );
	//printf( "FRAGMENT_SHADER:\n%s", fragmentSource.c_str() );

	const char* contentsPtr;

	GLint success;
	GLchar infoLog[512];
	
	// vertex shader compilation
	{
		contentsPtr = vertexSource.c_str();
		glShaderSource(m_VertexShaderID, 1, &contentsPtr, NULL);
		glCompileShader(m_VertexShaderID);

		glGetShaderiv(m_VertexShaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(m_VertexShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
		}
	}

	// geometry shader compilation
	if (m_GeometryShaderID != 0) {
		contentsPtr = geometrySource.c_str();
		glShaderSource(m_GeometryShaderID, 1, &contentsPtr, NULL);
		glCompileShader(m_GeometryShaderID);

		glGetShaderiv(m_GeometryShaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(m_GeometryShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::GEOMETRY_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
		}
	}

	// fragment shader compilation
	{
		contentsPtr = fragmentSource.c_str();
		glShaderSource(m_FragmentShaderID, 1, &contentsPtr, NULL);
		glCompileShader(m_FragmentShaderID);

		glGetShaderiv(m_FragmentShaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(m_FragmentShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
		}
	}
}

void Shader::compileShaders(const char* vertexPath, const char* fragmentPath) {
	m_Program = glCreateProgram();

	m_VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	m_FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	compileShader(m_VertexShaderID, vertexPath);
	compileShader(m_FragmentShaderID, fragmentPath);
}

void Shader::linkShaders() {
	GLint success;
	GLchar infoLog[512];

	if (m_VertexShaderID) {
		glAttachShader(m_Program, m_VertexShaderID);
	}
	if (m_GeometryShaderID) {
		glAttachShader(m_Program, m_GeometryShaderID);
	}
	if (m_FragmentShaderID) {
		glAttachShader(m_Program, m_FragmentShaderID);
	}
	glLinkProgram(m_Program);

	glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_Program, 512, NULL, infoLog);
		std::cout << "Failed to link shader program!\n " << infoLog << std::endl;
	}

	if (m_VertexShaderID) {
		glDetachShader(m_Program, m_VertexShaderID);
		glDeleteShader(m_VertexShaderID);
	}
	if (m_GeometryShaderID) {
		glDetachShader(m_Program, m_GeometryShaderID);
		glDeleteShader(m_GeometryShaderID);
	}
	if (m_FragmentShaderID) {
		glDetachShader(m_Program, m_FragmentShaderID);
		glDeleteShader(m_FragmentShaderID);
	}
}

void Shader::reload() {
	using namespace std::filesystem;
	file_time_type currModTime = last_write_time(m_Path);
	if (currModTime == m_LastModTime)
		return;

	m_LastModTime = currModTime;

	if (m_Program) {
		glDeleteShader(m_Program);
		m_Program = 0;
	}

	compileShaders(m_Path.c_str(), true);
	linkShaders();
}

void Shader::compileShader(GLuint shader, const char* filePath) {
	std::string path = std::string("../src/Graphics/Shaders/") + filePath;
	std::ifstream file(path.c_str());
	if (file.fail()) {
		utils::printError("Failed to open %s\n", filePath);
	}
	else {
		std::string content = "";
		std::string line;

		while (std::getline(file, line))
		{
			content += line + "\n";
		}

		const char* contentsPtr = content.c_str();
		glShaderSource(shader, 1, &contentsPtr, NULL);
		glCompileShader(shader);

		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << filePath << infoLog << std::endl;
		}
	}
	file.close();
}