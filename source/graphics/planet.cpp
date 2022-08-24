#include "StarSystemSim/graphics/planet.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include <iostream>
#include <cmath>
#include <map>
#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace graphics {

	static std::vector<Mesh::VertexData> vertices;
	static std::vector<uint32_t> indices;

	static std::map<std::pair<uint32_t, uint32_t>, uint32_t> lookup;

	static std::map<std::string, uint64_t> loadedTextures;


	Planet::Planet(const char* name)
		: m_MainMesh(nullptr)
	{
		init();

		subdivide(5);
		calcUVs();
		fixUVs();
		calcNormalsTangents();

		m_MainMesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
		loadTextures(name);
	}

	Planet::~Planet() {
		m_MainMesh = nullptr;
	}

	void Planet::draw(Shader& shader, uint32_t renderMode) {
		setPos(this->body.pos);
		m_MainMesh->draw(shader, renderMode);
	}

	void Planet::setPos(const glm::vec3& position) {
		this->resetTransMat();
		m_MainMesh->resetTransMat();

		this->translate(position);
	}

	glm::vec3 Planet::getPos() {
		this->setPos(this->body.pos);
		return Object::getPos();
	}

	void Planet::translate(const glm::vec3& translation) {
		((Object*)this)->translate(translation);
		m_MainMesh->translate(translation);
		this->body.pos = Object::getPos();
	}

	void Planet::rotate(float angle, const glm::vec3& axis) {
		((Object*)this)->rotate(angle, axis);
		m_MainMesh->rotate(angle, axis);
	}

	void Planet::scale(const glm::vec3& sc) {
		((Object*)this)->scale(sc);
		m_MainMesh->scale(sc);
	}

	void Planet::subdivide(uint32_t depth) {
		std::vector<Mesh::VertexData> nVertices;
		std::vector<uint32_t> nIndices;

		for (int i = 0; i < depth; ++i) {
			for (uint64_t triangle = 0; triangle < (indices.size() + 1) / 3; ++triangle) {
				divideTriangle(nVertices, nIndices, indices[3*triangle], indices[3*triangle + 1], indices[3*triangle + 2]);
			}

			vertices.clear();
			indices.clear();
			lookup.clear();

			vertices = nVertices;
			indices = nIndices;

			nVertices.clear();
			nIndices.clear();
		}
	}

	void Planet::divideTriangle(std::vector<Mesh::VertexData>& nVertices, std::vector<uint32_t>& nIndices, int ind1, int ind2, int ind3) {
		// indices
		GLuint oi1, oi2, oi3;
		GLuint ii1, ii2, ii3;

		//GLuint startInd = nVertices.size();
		GLuint lastInd = nVertices.size();

		// outer (old) vertices
		oi1 = lookUpOrAdd(nVertices, std::pair<int, int>(ind1, -1));
		oi2 = lookUpOrAdd(nVertices, std::pair<int, int>(ind2, -1));
		oi3 = lookUpOrAdd(nVertices, std::pair<int, int>(ind3, -1));

		// inner (new) vertices
		ii1 = lookUpOrAdd(nVertices, std::pair<int, int>(ind1, ind2));
		ii2 = lookUpOrAdd(nVertices, std::pair<int, int>(ind2, ind3));
		ii3 = lookUpOrAdd(nVertices, std::pair<int, int>(ind3, ind1));

		// adding indices
		{
			nIndices.push_back(oi1);
			nIndices.push_back(ii1);
			nIndices.push_back(ii3);

			nIndices.push_back(oi2);
			nIndices.push_back(ii2);
			nIndices.push_back(ii1);
		
			nIndices.push_back(oi3);
			nIndices.push_back(ii3);
			nIndices.push_back(ii2);
		
			nIndices.push_back(ii1);
			nIndices.push_back(ii2);
			nIndices.push_back(ii3);
		}
	}



	void Planet::calcUVs() {
		const float PI = 3.14159265359f;

		for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
			float x, y, z;
			x = (*iter).pos.x;
			y = (*iter).pos.y;
			z = (*iter).pos.z;

			float radius = std::sqrt(x * x + y * y + z * z);
			float lat;

			// sina = y / radius
			// a = arcsin(y / radius)
			lat = -glm::asin(y / radius);


			float px = x > 0.0f ? x : -x;
			float pz = z > 0.0f ? z : -z;
			float lng;
			if (px == 0.0f || px == -0.0f)
				lng = glm::atan(INFINITY);
			else
				lng = glm::atan(pz / px);

			if (z >= 0.0f && x >= 0.0f) {
				lng = 0.5f * PI - lng;
			}
			else if (z <= 0.0f && x >= 0.0f) {
				lng += 0.5f * PI;
			}
			else if (z <= 0.0f && x <= 0.0f) {
				lng = 0.5f * PI - lng;
				lng += PI;
			}
			else if (z >= 0.0f && x < 0.0f) {
				lng += 1.5f*PI;
			}

			(*iter).uv.x = lng / (2.0f * PI);
			(*iter).uv.y = (lat + 0.5f*PI) / PI;
		}
	}

	void Planet::fixUVs() {
		for (size_t triangleInd = 0; triangleInd < (indices.size() + 1) / 3; ++triangleInd) {
			GLuint A = indices[triangleInd * 3 + 0];
			GLuint B = indices[triangleInd * 3 + 1];
			GLuint C = indices[triangleInd * 3 + 2];

			glm::vec3 uvA = glm::vec3(vertices[A].uv.x, vertices[A].uv.y, 0.0f);
			glm::vec3 uvB = glm::vec3(vertices[B].uv.x, vertices[B].uv.y, 0.0f);
			glm::vec3 uvC = glm::vec3(vertices[C].uv.x, vertices[C].uv.y, 0.0f);

			// fixing highest values of U coordinates
			glm::vec3 uvNormal = glm::cross(uvB - uvA, uvC - uvA);
			if (uvNormal.z < 0.0f) {
				if (uvA.x < 0.3f) {
					Mesh::VertexData vertexCopy = vertices[A];
					uvA.x += 1.0f;
					vertexCopy.uv = uvA;
					indices[triangleInd * 3 + 0] = vertices.size();
					vertices.push_back(vertexCopy);
				}
				if (uvB.x < 0.3f) {
					Mesh::VertexData vertexCopy = vertices[B];
					uvB.x += 1.0f;
					vertexCopy.uv = uvB;
					indices[triangleInd * 3 + 1] = vertices.size();
					vertices.push_back(vertexCopy);
				}
				if (uvC.x < 0.3f) {
					Mesh::VertexData vertexCopy = vertices[C];
					uvC.x += 1.0f;
					vertexCopy.uv = uvC;
					indices[triangleInd * 3 + 2] = vertices.size();
					vertices.push_back(vertexCopy);
				}
			}

			// fixing uvs at the poles
			if (uvA.y == 0.0f || uvA.y == 1.0f) {
				Mesh::VertexData vertexCopy = vertices[A];
				vertexCopy.uv = glm::vec2(0.5f*(uvB.x + uvC.x), uvA.y);
			
				if (uvA.x >= 1.0f) {
					vertices[A] = vertexCopy;
				}
				else {
					indices[triangleInd * 3 + 0] = vertices.size();
					vertices.push_back(vertexCopy);
				}
			}
			if (uvB.y == 0.0f || uvB.y == 1.0f) {
				Mesh::VertexData vertexCopy = vertices[B];
				vertexCopy.uv = glm::vec2(0.5f*(uvA.x + uvC.x), uvB.y);
			
				if (uvB.x >= 1.0f) {
					vertices[B] = vertexCopy;
				}
				else {
					indices[triangleInd * 3 + 1] = vertices.size();
					vertices.push_back(vertexCopy);
				}
			}
			if (uvC.y == 0.0f || uvC.y == 1.0f) {
				Mesh::VertexData vertexCopy = vertices[C];
				vertexCopy.uv = glm::vec2(0.5f*(uvA.x + uvB.x), uvC.y);
			
				if (uvC.x >= 1.0f) {
					vertices[C] = vertexCopy;
				}
				else {
					indices[triangleInd * 3 + 2] = vertices.size();
					vertices.push_back(vertexCopy);
				}
			}
		}
	}

	void Planet::calcNormalsTangents() {
		// normals
		for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
			(*iter).normal = glm::normalize((*iter).pos);
		}

		// tangents
		std::set<Mesh::VertexData*> readyVertices;
		for (size_t triangleInd = 0; triangleInd < (indices.size() + 1) / 3; ++triangleInd) {
			Mesh::VertexData* vertA = &vertices[indices[triangleInd * 3 + 0]];
			Mesh::VertexData* vertB = &vertices[indices[triangleInd * 3 + 1]];
			Mesh::VertexData* vertC = &vertices[indices[triangleInd * 3 + 2]];
			
			glm::vec3 dPos1 = vertB->pos - vertA->pos;
			glm::vec3 dPos2 = vertC->pos - vertA->pos;
			glm::vec2 dUV1 = vertB->uv - vertA->uv;
			glm::vec2 dUV2 = vertC->uv - vertA->uv;
			
			float f = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
			
			glm::vec3 tan;
			tan.x = f * (dUV2.y * dPos1.x - dUV1.y * dPos2.x);
			tan.y = f * (dUV2.y * dPos1.y - dUV1.y * dPos2.y);
			tan.z = f * (dUV2.y * dPos1.z - dUV1.y * dPos2.z);
			
			tan = glm::normalize(tan);

			auto setTan = [&readyVertices, tan](Mesh::VertexData* v) mutable {
				if (readyVertices.count(v) == 0) {
					// making the tangent vector orthogonal to the normal vector of a vertex
					glm::vec3 orthTan;
					orthTan = glm::normalize(tan - glm::dot(tan, v->normal) * v->normal);

					v->tan = orthTan;
					readyVertices.insert(v);
				}
			};
			
			setTan(vertA);
			setTan(vertB);
			setTan(vertC);
		}
	}


	uint64_t Planet::lookUpOrAdd(std::vector<Mesh::VertexData>& nVertices, std::pair<int, int> pair) {
		uint64_t lastInd = nVertices.size();

		if (pair.second > pair.first) {
			std::swap(pair.first, pair.second);
		}

		if (lookup.find(pair) == lookup.end()) {
			Mesh::VertexData vertex;
			if(pair.second == -1)
				vertex = vertices[pair.first];
			else {
				vertex.pos = glm::normalize(vertices[pair.first].pos + vertices[pair.second].pos);
			}

			nVertices.push_back(vertex);
			lookup[pair] = lastInd;
			return lastInd;
		}
		else {
			return lookup[pair];
		}
	}

	void Planet::init() {
		const float a = 0.525731112119133606;
		const float b = 0.850650808352039932;

		Mesh::VertexData vertex;
		vertex.pos = { -a, 0.0f, b }; // 0
		vertices.push_back(vertex);
		vertex.pos = { a, 0.0f, b };  // 1
		vertices.push_back(vertex);
		vertex.pos = { -a, 0.0f, -b };// 2
		vertices.push_back(vertex);
		vertex.pos = { a, 0.0f, -b }; // 3
		vertices.push_back(vertex);
		vertex.pos = { 0.0f, b, a };  // 4
		vertices.push_back(vertex);
		vertex.pos = { 0.0f, b, -a }; // 5
		vertices.push_back(vertex);
		vertex.pos = { 0.0f, -b, a }; // 6
		vertices.push_back(vertex);
		vertex.pos = { 0.0f, -b, -a };// 7
		vertices.push_back(vertex);
		vertex.pos = { b, a, 0.0f };  // 8
		vertices.push_back(vertex);
		vertex.pos = { b, -a, 0.0f }; // 9
		vertices.push_back(vertex);
		vertex.pos = { -b, a, 0.0f }; // 10
		vertices.push_back(vertex);
		vertex.pos = { -b, -a, 0.0f };// 11
		vertices.push_back(vertex);

		const size_t N_FACES = 20;
		uint32_t inds[N_FACES * 3] =
		{
			4,1,0,
			4,0,10,
			4,10,5,
			4,5,8,
			4,8,1,
			9,1,8,
			8,3,9,
			5,3,8,
			5,2,3,
			3,2,7,
			7,9,3,
			7,6,9,
			7,11,6,
			0,6,11,
			0,1,6,
			1,9,6,
			0,11,10,
			10,11,2,
			10,2,5,
			7,2,11,
		};

		for (int i = 0; i < N_FACES * 3; ++i) {
			indices.push_back(inds[i]);
		}
	}

	void Planet::loadTextures(const std::string& name) {
		std::string path = "models/" + name + "/";
		m_MainMesh->bindTexture(loadTexture(path + "diffuse.jpg", true), Mesh::TextureType::DIFFUSE);
		m_MainMesh->bindTexture(loadTexture(path + "specular.jpg", true), Mesh::TextureType::SPECULAR);
		m_MainMesh->bindTexture(loadTexture(path + "normal.jpg", true), Mesh::TextureType::NORMAL);
		m_MainMesh->bindTexture(loadTexture(path + "nightmap.jpg"), Mesh::TextureType::NIGHT);
		m_MainMesh->bindTexture(loadTexture(path + "waves_normal1.jpg"), Mesh::TextureType::OTHER1);
		m_MainMesh->bindTexture(loadTexture(path + "waves_normal2.jpg"), Mesh::TextureType::OTHER2);
	}

	uint64_t Planet::loadTexture(const std::string& path, bool required) {
		GLuint textureID = 0;

		if (loadedTextures.find(path.c_str()) != loadedTextures.end()) {
			return loadedTextures[path];
		}

		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
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

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if(required)
		{
			utils::printError("Failed to load texture: %s", path.c_str());
		}

		stbi_image_free(data);

		loadedTextures.insert(std::pair<std::string, GLuint>(path, textureID));
		utils::print("Loaded texture: %s", path.c_str());

		return textureID;
	}

}