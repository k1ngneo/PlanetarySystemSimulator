#pragma once

#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/shader.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <string>

namespace graphics {

	class Mesh : public Object {
	public:
		struct VertexData;

		Mesh(std::vector<VertexData>& vert, std::vector<unsigned int>& ind);
		~Mesh();
		void draw(Shader& shader);

		enum class TextureType {
			DIFFUSE, SPECULAR, NORMAL, NIGHT
		};

		void bindTexture(GLuint textureID, TextureType type);

		inline const std::vector<VertexData>& getVertices() { return m_Vertices; }
		inline const std::vector<GLuint>& getIndices() { return m_Indices; }

		void buildMesh(std::vector<VertexData>& vert, std::vector<GLuint>& ind);

	private:
		std::vector<VertexData> m_Vertices;
		std::vector<GLuint> m_Indices;
		GLuint m_DiffuseTex, m_SpecularTex, m_NormalTex, m_NightTex;

		GLuint m_VAO, m_VBO, m_EBO;

		void setUpMesh();
	};

	struct Mesh::VertexData {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tan;
	};
}