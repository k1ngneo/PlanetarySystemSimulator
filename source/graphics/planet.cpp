#include "StarSystemSim/graphics/planet.h"

#include <glm/vec3.hpp>

#include <iostream>
#include <map>

namespace graphics {

	std::vector<Mesh::VertexData> vertices;
	std::vector<GLuint> indices;

	std::map<std::pair<GLuint, GLuint>, GLuint> lookup;


	Planet::Planet(const char* name)
		: Model(name)
	{
		Mesh& globMesh = getMesh(0);
		vertices = globMesh.getVertices();
		indices = globMesh.getIndices();

		subdivide(5);

		calcUVs(vertices);
		calcNormals(vertices);

		globMesh.buildMesh(vertices, indices);
	}

	Planet::~Planet() {
		this->Model::~Model();
	}

	void Planet::subdivide(int depth) {
		std::vector<Mesh::VertexData> nVertices;
		std::vector<GLuint> nIndices;

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

	void Planet::divideTriangle(std::vector<Mesh::VertexData>& nVertices, std::vector<GLuint>& nIndices, int ind1, int ind2, int ind3) {
		// indices
		GLuint oi1, oi2, oi3;
		GLuint ii1, ii2, ii3;

		//GLuint startInd = nVertices.size();
		GLuint lastInd = nVertices.size();

		// outer (old) vertices
		oi1 = lookUpOrAdd(nVertices, std::pair<int, int>(ind1, -1));
		oi2 = lookUpOrAdd(nVertices, std::pair<int, int>(ind2, -1));
		oi3 = lookUpOrAdd(nVertices, std::pair<int, int>(ind3, -1));

		ii1 = lookUpOrAdd(nVertices, std::pair<int, int>(ind1, ind2));
		ii2 = lookUpOrAdd(nVertices, std::pair<int, int>(ind2, ind3));
		ii3 = lookUpOrAdd(nVertices, std::pair<int, int>(ind3, ind1));

		// adding indices
		{
			nIndices.push_back(oi1);
			nIndices.push_back(ii3);
			nIndices.push_back(ii1);

			nIndices.push_back(oi2);
			nIndices.push_back(ii1);
			nIndices.push_back(ii2);
		
			nIndices.push_back(oi3);
			nIndices.push_back(ii2);
			nIndices.push_back(ii3);
		
			nIndices.push_back(ii1);
			nIndices.push_back(ii3);
			nIndices.push_back(ii2);
		}
	}



	void Planet::calcUVs(std::vector<Mesh::VertexData>& vertices) {
		const float PI = 3.14159265359f;

		float biggest_lng = -INFINITY;
		float smallest_lng = INFINITY;
		float biggest_lat = -INFINITY;
		float smallest_lat = INFINITY;

		for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
			float x, y, z;
			x = (*iter).pos.x;
			y = (*iter).pos.y;
			z = (*iter).pos.z;

			float radius = std::sqrt(x * x + z * z);
			float lat;
			if (y == 1.0f)
				lat = 0.5f * PI;
			else if (y == -1.0f)
				lat = -0.5f * PI;
			else {
				lat = -glm::atan((*iter).pos.y / radius);
			}


			float px = x > 0.0f ? x : -x;
			float pz = z > 0.0f ? z : -z;
			float lng;
			if (px == 0.0f || px == -0.0f)
				lng = glm::atan(INFINITY);
			else
				lng = glm::atan(pz / px);

			lng = lng > 0.0f ? lng : PI-lng;

			if (z >= 0.0f && x >= 0.0f) {
				lng += 1.5f*lng;
			}
			else if (z >= 0.0f && x <= 0.0f) {
				lng += 1.5f*PI;
			}
			else if (z <= 0.0f && x <= 0.0f) {
				lng += PI;
			}
			else if (z <= 0.0f && x >= 0.0f) {
				lng += 0.5f * PI;
			}

			(*iter).uv.x = (lng / (2.0f * PI)) * glm::cos(PI);
			(*iter).uv.y = (lat + 0.5f*PI) / PI;

			//(*iter).uv.x = 0.0f;
			//(*iter).uv.y = 0.0f;

			biggest_lng = std::max(biggest_lng, lng);
			smallest_lng = std::min(smallest_lng, lng);
			biggest_lat = std::max(biggest_lat, lat);
			smallest_lat = std::min(smallest_lat, lat);
		}

		std::cout << "smallest lng: " << smallest_lng << " | biggest lng: " << biggest_lng << "\n";
		std::cout << "smallest lat: " << smallest_lat << " | biggest lat: " << biggest_lat << "\n";
	}

	void Planet::calcNormals(std::vector<Mesh::VertexData>& vertices) {
		for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
			(*iter).normal = glm::normalize((*iter).pos);
		}
	}


	GLuint Planet::lookUpOrAdd(std::vector<Mesh::VertexData>& nVertices, std::pair<int, int> pair) {
		GLuint lastInd = nVertices.size();

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

}