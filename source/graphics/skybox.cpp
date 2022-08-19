#include "StarSystemSim/graphics/skybox.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>

#include <stb_image.h>

namespace graphics {

	Skybox::Skybox()
		: m_VAO(0), m_VBO(0), m_EBO(0),
		m_CubeMapTex(0)
	{
		initVAO();
	}

	Skybox::~Skybox() {
		if (m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (m_EBO)
			glDeleteBuffers(1, &m_EBO);
	}

	void Skybox::draw(Shader& shader) {
		shader.use();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTex);
		shader.setUniform1i("_cubeMap", 0);

		glDepthMask(GL_FALSE);
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		shader.unuse();
	}

	void Skybox::loadTexture(const std::string& path) {
		if (!m_CubeMapTex)
			glGenTextures(1, &m_CubeMapTex);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTex);

		auto loadTexture = [](const std::string& path, GLuint side) {
			int width, height, nrChannels;
			unsigned char* data = nullptr;
			data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				glTexImage2D(side, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else {
				utils::printError("Failed to load skybox texture: %s", path.c_str());
			}
		};

		loadTexture(path + "cubemap+x.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		loadTexture(path + "cubemap-x.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		loadTexture(path + "cubemap+y.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		loadTexture(path + "cubemap-y.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		loadTexture(path + "cubemap+z.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		loadTexture(path + "cubemap-z.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	void Skybox::initVAO() {
		glm::vec3 vertices[] = {
			{-1.0f,  1.0f,  1.0f}, //     4-----------5
			{ 1.0f,  1.0f,  1.0f}, //    /|          /|
			{-1.0f, -1.0f,  1.0f}, //   0-+---------1 |
			{ 1.0f, -1.0f,  1.0f}, //   | |         | |
			{-1.0f,  1.0f, -1.0f}, //   | |         | |
			{ 1.0f,  1.0f, -1.0f}, //   | 6---------+-7
			{-1.0f, -1.0f, -1.0f}, //   |/          |/
			{ 1.0f, -1.0f, -1.0f}, //   2-----------3
		};

		GLuint indices[] = {
			// counter-clock-wise
			0, 2, 1, 1, 2, 3, // front face
			1, 3, 5, 5, 3, 7, // right face
			5, 7, 4, 4, 7, 6, // back face
			4, 6, 0, 0, 6, 2, // left face
			0, 1, 4, 1, 5, 4, // upper face
			6, 3, 2, 6, 7, 3, // bottom face
		};

		if (!m_VAO)
			glGenVertexArrays(1, &m_VAO);
		if (!m_VBO)
			glGenBuffers(1, &m_VBO);
		if (!m_EBO)
			glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);
		
		glBindVertexArray(0);
	}

}