#pragma once

#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/shader.h"

#include <glm/vec3.hpp>

namespace graphics { namespace primitives {

	class PointLight : public Object {
	public:
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 attenuation;

		void draw(Shader& shader);

		PointLight();
		~PointLight();

		static void bindVAO();
		static void unbindVAO();

		static void initVBO();
		static void destroyVBO();

		struct VertexData {
			glm::vec3 pos;
		};

		void translate(const glm::vec3& trans);

		inline const glm::vec3& getPos() { return _pos; }
		void setPos(const glm::vec3& pos);

	private:
		glm::vec3 _pos;

		static unsigned int _VAO, _VBO;
		static const unsigned int _VERTICES_COUNT = 36;
		static VertexData _vertices[_VERTICES_COUNT * sizeof(VertexData)];
	};

} }