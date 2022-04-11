#pragma once

#include "StarSystemSim/graphics/shader.h"

#include <glad/glad.h>
#include <string>

namespace graphics {

	class Skybox {
	public:
		Skybox();
		~Skybox();

		void draw(Shader& shader);

	private:
		GLuint m_VAO, m_VBO, m_EBO;
		GLuint m_CubeMapTex;

		void initVAO();
		void loadTexture(const std::string& path);
	};

}