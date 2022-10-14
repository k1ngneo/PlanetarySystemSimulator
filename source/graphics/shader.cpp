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
	: m_Program(0), m_AttributeNum(0)
{
}

Shader::~Shader() {
	if (m_Program != 0)
		glDeleteProgram(m_Program);
}

void Shader::use() {
	glUseProgram(m_Program);
}
void Shader::unuse() {
	glUseProgram(0);
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

void Shader::buildShaders(const char* src, bool path) {
	std::string source = src;

	if (path) {
		m_Path = src;
		source = utils::loadTextFile(src);
		m_LastModTime = std::filesystem::last_write_time(src);
	}

	m_Program = glCreateProgram();

	uint32_t vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	uint32_t fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	uint32_t controlShaderID = 0;
	uint32_t evaluationShaderID = 0;
	uint32_t geometryShaderID = 0;

	std::string vertexSource = "";
	std::string controlSource = "";
	std::string evaluationSource = "";
	std::string geometrySource = "";
	std::string fragmentSource = "";
	std::string line;

	enum class ShaderType { VERTEX_SHADER, CONTROL_SHADER, EVALUATION_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER, NO_SHADER } shader_type;
	shader_type = ShaderType::NO_SHADER;

	const std::string vertexShaderPrep = "#vertex_shader";
	const std::string controlShaderPrep = "#control_shader";
	const std::string evaluationShaderPrep = "#evaluation_shader";
	const std::string geometryShaderPrep = "#geometry_shader";
	const std::string fragmentShaderPrep = "#fragment_shader";

	// reading a file
	{
		for (unsigned int i = 0; i < utils::howManyLines(source.c_str()); ++i) {
			line = utils::getLine(source.c_str(), i);

			bool isItPrepCom = false;

			if (!line.compare(vertexShaderPrep)) {
				shader_type = ShaderType::VERTEX_SHADER;
				isItPrepCom = true;
			}
			else if (!line.compare(controlShaderPrep)) {
				shader_type = ShaderType::CONTROL_SHADER;
				isItPrepCom = true;

				if (controlShaderID == 0)
					controlShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
			}
			else if (!line.compare(evaluationShaderPrep)) {
				shader_type = ShaderType::EVALUATION_SHADER;
				isItPrepCom = true;

				if (evaluationShaderID == 0)
					evaluationShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
			}
			else if (!line.compare(geometryShaderPrep)) {
				shader_type = ShaderType::GEOMETRY_SHADER;
				isItPrepCom = true;

				if (geometryShaderID == 0)
					geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
			}
			else if (!line.compare(fragmentShaderPrep)) {
				shader_type = ShaderType::FRAGMENT_SHADER;
				isItPrepCom = true;
			}


			if (!isItPrepCom) {
				switch (shader_type) {
				case ShaderType::VERTEX_SHADER:
				{
					vertexSource += line + "\n";
				} break;
				case ShaderType::CONTROL_SHADER:
				{

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
	}

	//printf( "VERTEX_SHADER:\n%s", vertexSource.c_str() );
	//printf( "CONTROL_SHADER:\n%s", controlSource.c_str() );
	//printf( "EVALUATION_SHADER:\n%s", evaluationSource.c_str() );
	//printf( "GEOMETRY_SHADER:\n%s", geometrySource.c_str() );
	//printf( "FRAGMENT_SHADER:\n%s", fragmentSource.c_str() );

	const char* contentsPtr;

	GLint success;
	GLchar infoLog[512];
	
	// compiling shaders
	{
		// vertex shader compilation
		{
			contentsPtr = vertexSource.c_str();
			glShaderSource(vertexShaderID, 1, &contentsPtr, NULL);
			glCompileShader(vertexShaderID);

			glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
				std::cout << "Shader: " << src << '\n';
				std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
			}

			glAttachShader(m_Program, vertexShaderID);
		}

		// control shader compilation
		if (controlShaderID != 0) {
			contentsPtr = controlSource.c_str();
			glShaderSource(controlShaderID, 1, &contentsPtr, NULL);
			glCompileShader(controlShaderID);

			glGetShaderiv(controlShaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(controlShaderID, 512, NULL, infoLog);
				std::cout << "Shader: " << src << '\n';
				std::cout << "ERROR::CONTROL_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
			}

			glAttachShader(m_Program, controlShaderID);
		}

		// evaluation shader compilation
		if (evaluationShaderID != 0) {
			contentsPtr = evaluationSource.c_str();
			glShaderSource(evaluationShaderID, 1, &contentsPtr, NULL);
			glCompileShader(evaluationShaderID);

			glGetShaderiv(evaluationShaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(evaluationShaderID, 512, NULL, infoLog);
				std::cout << "Shader: " << src << '\n';
				std::cout << "ERROR::EVALUATION_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
			}

			glAttachShader(m_Program, evaluationShaderID);
		}

		// geometry shader compilation
		if (geometryShaderID != 0) {
			contentsPtr = geometrySource.c_str();
			glShaderSource(geometryShaderID, 1, &contentsPtr, NULL);
			glCompileShader(geometryShaderID);

			glGetShaderiv(geometryShaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(geometryShaderID, 512, NULL, infoLog);
				std::cout << "Shader: " << src << '\n';
				std::cout << "ERROR::GEOMETRY_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
			}

			glAttachShader(m_Program, geometryShaderID);
		}

		// fragment shader compilation
		{
			contentsPtr = fragmentSource.c_str();
			glShaderSource(fragmentShaderID, 1, &contentsPtr, NULL);
			glCompileShader(fragmentShaderID);

			glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
				std::cout << "Shader: " << src << '\n';
				std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
			}

			glAttachShader(m_Program, fragmentShaderID);
		}
	}

	// linking shaders
	glLinkProgram(m_Program);

	if (vertexShaderID) {
		glDetachShader(m_Program, vertexShaderID);
		glDeleteShader(vertexShaderID);
	}
	if (controlShaderID) {
		glDetachShader(m_Program, controlShaderID);
		glDeleteShader(controlShaderID);
	}
	if (evaluationShaderID) {
		glDetachShader(m_Program, evaluationShaderID);
		glDeleteShader(evaluationShaderID);
	}
	if (geometryShaderID) {
		glDetachShader(m_Program, geometryShaderID);
		glDeleteShader(geometryShaderID);
	}
	if (fragmentShaderID) {
		glDetachShader(m_Program, fragmentShaderID);
		glDeleteShader(fragmentShaderID);
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

	buildShaders(m_Path.c_str(), true);
}

void Shader::compileShader(uint32_t shader, const char* filePath) {
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