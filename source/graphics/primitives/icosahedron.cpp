#include "StarSystemSim/graphics/primitives/icosahedron.h"

namespace graphics { namespace primitives {

	Icosahedron::Icosahedron()
		: m_Vertices(nullptr), m_Indices(nullptr),
		m_VAO(0), m_VBO(0), m_EBO(0)
	{
		init();
	}

	Icosahedron::~Icosahedron() {
		if (m_Vertices)
			delete m_Vertices;
		if (m_Indices)
			delete m_Indices;

		if (m_VAO)
			glDeleteVertexArrays(1, &m_VAO);

		if (m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (m_EBO)
			glDeleteBuffers(1, &m_EBO);
	}

	void Icosahedron::draw(Shader& shader) {
		this->updateModelMat();

		shader.setUniformMat4("_modelMat", this->modelMat);

		shader.use();
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, sizeof(m_Indices), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		shader.unuse();
	}

	void Icosahedron::init() {
		m_Vertices = new VertexData[12];
		const float a = 0.525731112119133606;
		const float b = 0.850650808352039932;

		m_Vertices[0].pos = { -a, 0.0f, b };
		m_Vertices[1].pos = { a, 0.0f, b };
		m_Vertices[2].pos = { -a, 0.0f, -b };
		m_Vertices[3].pos = { a, 0.0f, -b };
		m_Vertices[4].pos = { 0.0f, b, a };
		m_Vertices[5].pos = { 0.0f, b, -a };
		m_Vertices[6].pos = { 0.0f, -b, a };
		m_Vertices[7].pos = { 0.0f, -b, -a };
		m_Vertices[8].pos = { b, a, 0.0f };
		m_Vertices[9].pos = { b, -a, 0.0f };
		m_Vertices[10].pos = { -b, a, 0.0f };
		m_Vertices[11].pos = { -b, -a, 0.0f };

		m_Indices = new glm::vec3[20];

		{
			m_Indices[0] = { 0,1,4 };
			m_Indices[1] = { 0,4,9 };
			m_Indices[2] = { 9,4,5 };
			m_Indices[3] = { 4,8,5 };
			m_Indices[4] = { 4,1,8 };
			m_Indices[5] = { 8,1,10 };
			m_Indices[6] = { 8,10,3 };
			m_Indices[7] = { 5,8,3 };
			m_Indices[8] = { 5,3,2 };
			m_Indices[9] = { 2,3,7 };
			m_Indices[10] = { 7,3,10 };
			m_Indices[11] = { 7,10,6 };
			m_Indices[12] = { 7,6,11 };
			m_Indices[13] = { 11,6,0 };
			m_Indices[14] = { 0,6,1 };
			m_Indices[15] = { 6,10,1 };
			m_Indices[16] = { 9,11,0 };
			m_Indices[17] = { 9,2,11 };
			m_Indices[18] = { 9,5,2 };
			m_Indices[19] = { 7,11,2 };
		}

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices), m_Vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, pos));

		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices), m_Indices, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

} }