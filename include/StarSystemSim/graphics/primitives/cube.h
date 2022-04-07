#pragma once

#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/material.h"

namespace graphics { namespace primitives {

	class Cube : public Object {
	public:
		Material material;

		void draw();

		Cube();
		~Cube();

		static void bindVAO();
		static void unbindVAO();

		static void initVBO();
		static void destroyVBO();

		struct VertexData {
			glm::vec3 pos;
			glm::vec3 normal;
		};

	private:
		static unsigned int _VAO, _VBO;
		static const unsigned int _VERTICES_COUNT = 36;
		static VertexData _vertices[_VERTICES_COUNT * sizeof(VertexData)];
	};

} }