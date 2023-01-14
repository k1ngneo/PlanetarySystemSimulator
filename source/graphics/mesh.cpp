#include "StarSystemSim/graphics/mesh.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>

namespace graphics {

	Mesh::Mesh(std::vector<VertexData>& vert, std::vector<uint32_t>& ind)
		: m_VAO(0), m_VBO(0), m_EBO(0),
		m_DiffuseTex(0), m_SpecHeightTex(0), m_NightTex(0),
		m_Other1Tex(0), m_Other2Tex(0)
	{
		buildMesh(vert, ind);
	}

	Mesh::~Mesh() {
		if (m_DiffuseTex)
			glDeleteTextures(1, &m_DiffuseTex);
		if (m_SpecHeightTex)
			glDeleteTextures(1, &m_SpecHeightTex);
		if (m_NightTex)
			glDeleteTextures(1, &m_NightTex);
		if (m_Other1Tex)
			glDeleteTextures(1, &m_Other1Tex);

		m_DiffuseTex = 0;
		m_SpecHeightTex = 0;
		m_NightTex = 0;
		m_Other1Tex = 0;
		m_Other2Tex = 0;
		
		if(m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (m_EBO)
			glDeleteBuffers(1, &m_EBO);
		
		m_VAO = m_VBO = m_EBO = 0;
	}

	void Mesh::draw(Shader& shader, uint32_t renderMode) {
		updateModelMat();

		shader.use();
		shader.setUniformMat4("_modelMat", this->modelMat);
		shader.setUniform1i("_diffTex", 0);
		shader.setUniform1i("_specHeightTex", 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_DiffuseTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_SpecHeightTex);

		if (m_NightTex) {
			shader.setUniform1i("_nightTex", 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, m_NightTex);
		}
		if (m_Other1Tex) {
			shader.setUniform1i("_otherTex1", 4);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_Other1Tex);
		}
		if (m_Other2Tex) {
			shader.setUniform1i("_otherTex2", 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, m_Other2Tex);
		}

		glBindVertexArray(m_VAO);
		glDrawElements(renderMode, m_Indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		shader.unuse();
	}

	void Mesh::bindTexture(uint64_t textureID, TextureType type)
	{
		switch (type) {
			case TextureType::DIFFUSE:
				if(!m_DiffuseTex)
					m_DiffuseTex = textureID;
				break;
			case TextureType::SPEC_HEIGHT:
				if (!m_SpecHeightTex)
					m_SpecHeightTex = textureID;
				break;
			case TextureType::NIGHT:
				if (!m_NightTex)
					m_NightTex = textureID;
				break;
			case TextureType::OTHER1:
				if (!m_Other1Tex)
					m_Other1Tex = textureID;
				break;
			case TextureType::OTHER2:
				if (!m_Other2Tex)
					m_Other2Tex = textureID;
				break;
			default:
				break;
		}
	}

	void Mesh::buildMesh(std::vector<VertexData>& vert, std::vector<uint32_t>& ind) {
		m_Vertices = std::move(vert);
		m_Indices = std::move(ind);

		setUpMesh();
	}

	void Mesh::setUpMesh() {
		if(!m_VAO)
			glGenVertexArrays(1, &m_VAO);
		if(!m_VBO)
			glGenBuffers(1, &m_VBO);
		if(!m_EBO)
			glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(VertexData), &m_Vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, pos));
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, tan));

		glBindVertexArray(0);
	}

}