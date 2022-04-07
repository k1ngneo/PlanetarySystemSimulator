#include "StarSystemSim/graphics/object.h"

#include "StarSystemSim/graphics/shader.h"

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace graphics { namespace primitives {

	class Icosahedron : public Object {
	public:
		Icosahedron();
		~Icosahedron();

		void draw(Shader& shader);

		struct VertexData {
			glm::vec3 pos;
			glm::vec2 uv;
		};

	private:
		void init();

		VertexData* m_Vertices;
		glm::vec3* m_Indices;
		GLuint m_VAO, m_VBO, m_EBO;
	};

} }