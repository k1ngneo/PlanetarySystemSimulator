#include "StarSystemSim/graphics/primitives/plane.h"

#include <glad/glad.h>

namespace graphics { namespace primitives {

	void Plane::draw(Shader& shader) {
		updateModelMat();

		shader.use();
		shader.setUniformMat4("_modelMat", this->modelMat);

		glBindVertexArray(_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		shader.unuse();
	}

	Plane::Plane() {
		initVAO();
	}

	void Plane::initVAO() {
		VertexData vertices[4];
		vertices[0].pos = { -0.5f, 0.0f,  0.5f };
		vertices[1].pos = {  0.5f, 0.0f,  0.5f };
		vertices[2].pos = { -0.5f, 0.0f, -0.5f };
		vertices[3].pos = {  0.5f, 0.0f, -0.5f };

		vertices[0].uv = { 0.0f, 1.0f };
		vertices[1].uv = { 1.0f, 1.0f };
		vertices[2].uv = { 0.0f, 0.0f };
		vertices[3].uv = { 1.0f, 0.0f };

		for (VertexData& vertex : vertices) {
			vertex.normal = { 0.0f, 0.0f, 1.0f };
			vertex.tan = { 1.0f, 0.0f, 0.0f };
		}
		

		unsigned int indices[] = {
			0, 1, 2,
			1, 3, 2,
		};

		glGenVertexArrays(1, &_VAO);
		glGenBuffers(1, &_VBO);
		glGenBuffers(1, &_EBO);

		glBindVertexArray(_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, VertexData::pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, VertexData::uv));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, VertexData::normal));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, VertexData::tan));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

} }