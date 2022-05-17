#pragma once

#include "StarSystemSim/graphics/shader.h"

#include <string>

namespace graphics {

	class Skybox {
	public:
		Skybox();
		~Skybox();

		void draw(Shader& shader);

	private:
		unsigned int m_VAO, m_VBO, m_EBO;
		unsigned int m_CubeMapTex;

		void initVAO();
		void loadTexture(const std::string& path);
	};

}