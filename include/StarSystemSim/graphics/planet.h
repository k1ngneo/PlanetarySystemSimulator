#pragma once

#include "StarSystemSim/graphics/mesh.h"
#include "StarSystemSim/physics/body.h"

#include <string>

namespace graphics {

	class Planet : public Object {
	public:
		Planet(const char* name);
		~Planet();

		void draw(Shader& shader);

		void setPos(const glm::vec3& position);

		void translate(const glm::vec3& translation);
		void rotate(float angle, const glm::vec3& axis);
		void scale(const glm::vec3& sc);

		void activateBody(physics::Engine& engine);

		physics::Body m_Body;
	private:
		void subdivide(int depth = 1);
		void divideTriangle(std::vector<Mesh::VertexData>& nVertices, std::vector<GLuint>& nIndices, int ind1, int ind2, int ind3);
		void calcUVs();
		void fixUVs();
		void calcNormalsTangents();

		GLuint lookUpOrAdd(std::vector<Mesh::VertexData>& nVertices, std::pair<int, int> pair);
		
		void init();
		void loadTextures(const std::string& name);
		GLuint loadTexture(const std::string& path, bool required = false);

		Mesh* m_MainMesh;
	};

}