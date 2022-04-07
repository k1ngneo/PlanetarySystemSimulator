#include "StarSystemSim/graphics/mesh.h"

#include "StarSystemSim/utilities/error.h"

namespace graphics {

	Mesh::Mesh(std::vector<VertexData>& vert, std::vector<unsigned int>& ind)
		: m_VAO(0), m_VBO(0), m_EBO(0),
		m_DiffuseTex(0), m_SpecularTex(0), m_NormalTex(0)
	{
		buildMesh(vert, ind);
	}

	Mesh::~Mesh() {
		if(m_DiffuseTex)
			glDeleteTextures(1, &m_DiffuseTex);
		if(m_SpecularTex)
			glDeleteTextures(1, &m_SpecularTex);
		if(m_NormalTex)
			glDeleteTextures(1, &m_NormalTex);

		m_DiffuseTex = 0;
		m_SpecularTex = 0;
		m_NormalTex = 0;
		
		if(m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (m_EBO)
			glDeleteBuffers(1, &m_EBO);
		
		m_VAO = m_VBO = m_EBO = 0;
	}

	void Mesh::draw(Shader& shader) {
		updateModelMat();

		shader.use();
		shader.setUniformMat4("_modelMat", this->modelMat);
		shader.setUniform1i("_diffTex", 0);
		shader.setUniform1i("_specTex", 1);
		shader.setUniform1i("_normTex", 2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_DiffuseTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_SpecularTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_NormalTex);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		shader.unuse();
	}

	void Mesh::bindTexture(GLuint textureID, TextureType type)
	{
		switch (type) {
			case TextureType::DIFFUSE:
				if(!m_DiffuseTex)
					m_DiffuseTex = textureID;
				break;
			case TextureType::SPECULAR:
				if (!m_SpecularTex)
					m_SpecularTex = textureID;
				break;
			case TextureType::NORMAL:
				if (!m_NormalTex)
					m_NormalTex = textureID;
			default:
				break;
		}
	}

	void Mesh::buildMesh(std::vector<VertexData>& vert, std::vector<GLuint>& ind) {
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