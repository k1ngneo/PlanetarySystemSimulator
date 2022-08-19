#pragma once

#include "StarSystemSim/graphics/mesh.h"
#include "StarSystemSim/physics/body.h"

#include <glad/glad.h>

#include <string>
#include <memory>

namespace graphics {

	class Planet : public Object {
	public:
		Planet(const char* name);
		~Planet();

		void draw(Shader& shader, uint32_t renderMode = GL_TRIANGLES) override;

		void setPos(const glm::vec3& position);
		glm::vec3 getPos();

		void translate(const glm::vec3& translation);
		void rotate(float angle, const glm::vec3& axis);
		void scale(const glm::vec3& sc);

		physics::Body body;

	private:
		void subdivide(uint32_t depth = 1);
		void divideTriangle(std::vector<Mesh::VertexData>& nVertices, std::vector<uint32_t>& nIndices, int ind1, int ind2, int ind3);
		void calcUVs();
		void fixUVs();
		void calcNormalsTangents();

		uint64_t lookUpOrAdd(std::vector<Mesh::VertexData>& nVertices, std::pair<int, int> pair);
		
		void init();
		void loadTextures(const std::string& name);
		uint64_t loadTexture(const std::string& path, bool required = false);

		std::shared_ptr<Mesh> m_MainMesh;
	};

}