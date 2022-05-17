#include "StarSystemSim/graphics/model.h"

#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <iostream>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace graphics {

	Model::Model(const char* path)
	{
		loadModel(path);
	}

	Model::~Model() {
		for (auto iter = m_Meshes.begin(); iter != m_Meshes.end(); ++iter) {
			delete (*iter);
		}
	}

	void Model::draw(Shader& shader) {
		for (Mesh* mesh : m_Meshes) {
			mesh->draw(shader);
		}
	}

	void Model::translate(const glm::vec3& trans) {
		for (Mesh* mesh : m_Meshes) {
			mesh->translate(trans);
		}
	}
	void Model::rotate(float angle, const glm::vec3& axis) {
		for (Mesh* mesh : m_Meshes) {
			mesh->rotate(angle, axis);
		}
	}
	void Model::scale(const glm::vec3& scale) {
		for (Mesh* mesh : m_Meshes) {
			mesh->scale(scale);
		}
	}

	void Model::loadModel(const std::string& name) {
		std::string path = "models/";
		path += name + "/";

		Assimp::Importer imp;
		unsigned int readFlags;
		readFlags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes
			| aiProcess_GenNormals | aiProcess_CalcTangentSpace;
		const aiScene* scene = imp.ReadFile(path + name + ".obj", readFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "ERROR::ASSIMP::" << imp.GetErrorString() << '\n';
			return;
		}
		m_Path = path.substr(0, path.find_last_of('/'));

		processAssimpNode(scene->mRootNode, scene);

		GLuint diffuseTex = loadTexture(path + "diffuse.jpg");
		GLuint specularTex = loadTexture(path + "specular.jpg");
		GLuint normalTex = loadTexture(path + "normal.jpg");

		for(Mesh* mesh : m_Meshes) {
			mesh->bindTexture(diffuseTex, Mesh::TextureType::DIFFUSE);
			mesh->bindTexture(specularTex, Mesh::TextureType::SPECULAR);
			mesh->bindTexture(normalTex, Mesh::TextureType::NORMAL);
		}
	}

	GLuint Model::loadTexture(const std::string& path) {
		GLuint textureID = 0;

		glGenTextures(1, &textureID);
		if (!textureID) {
			utils::fatalError("Failed generating texture");
		}
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//stbi_set_flip_vertically_on_load(true);

		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			utils::printError("Failed to load texture: %s\n", path);
		}

		stbi_image_free(data);

		return textureID;
	}

	void Model::processAssimpNode(aiNode* node, const aiScene* scene) {
		for (unsigned int meshInd = 0; meshInd < node->mNumMeshes; ++meshInd) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[meshInd]];
			m_Meshes.push_back(processAssimpMesh(mesh, scene));
		}

		for (unsigned int childInd = 0; childInd < node->mNumChildren; ++childInd) {
			processAssimpNode(node->mChildren[childInd], scene);
		}
	}

	Mesh* Model::processAssimpMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<Mesh::VertexData> vertices;
		std::vector<uint32_t> indices;

		for (unsigned int vertInd = 0; vertInd < mesh->mNumVertices; ++vertInd) {
			Mesh::VertexData vertex;
			vertex.pos.x = mesh->mVertices[vertInd].x;
			vertex.pos.y = mesh->mVertices[vertInd].y;
			vertex.pos.z = mesh->mVertices[vertInd].z;

			if (mesh->HasNormals()) {
				vertex.normal.x = mesh->mNormals[vertInd].x;
				vertex.normal.y = mesh->mNormals[vertInd].y;
				vertex.normal.z = mesh->mNormals[vertInd].z;
			}

			if (mesh->HasTangentsAndBitangents()) {
				vertex.tan.x = mesh->mTangents[vertInd].x;
				vertex.tan.y = mesh->mTangents[vertInd].y;
				vertex.tan.z = mesh->mTangents[vertInd].z;
			}

			if (mesh->mTextureCoords[0]) {
				vertex.uv.x = mesh->mTextureCoords[0][vertInd].x;
				vertex.uv.y = mesh->mTextureCoords[0][vertInd].y;
			}
			else {
				vertex.uv = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		for (unsigned int triangleInd = 0; triangleInd < mesh->mNumFaces; ++triangleInd) {
			aiFace face = mesh->mFaces[triangleInd];
			for (unsigned int i = 0; i < face.mNumIndices; ++i) {
				indices.push_back(face.mIndices[i]);
			}
		}

		return new Mesh(vertices, indices);
	}

}