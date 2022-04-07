#pragma once

#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/mesh.h"

#include <glm/vec3.hpp>

#include <assimp/scene.h>

#include <string>
#include <vector>

namespace graphics {

	class Model {
	public:
		Model(const char* path);
		~Model();

		void draw(Shader& shader);

		void translate(const glm::vec3& trans);
		void rotate(float angle, const glm::vec3& axis);
		void scale(const glm::vec3& scale);

		inline Mesh& getMesh(int meshID) { return *(m_Meshes[meshID]); }

	private:
		std::vector<Mesh*> m_Meshes;
		std::string m_Path;

		void loadModel(const std::string& path);
		GLuint loadTexture(const std::string& path);

		void processAssimpNode(aiNode* node, const aiScene* scene);
		Mesh* processAssimpMesh(aiMesh* mesh, const aiScene* scene);
	};

}