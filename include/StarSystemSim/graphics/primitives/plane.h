#pragma once

#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/shader.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace graphics {
	namespace primitives {

		class Plane : public Object {
		public:
			void draw(Shader& shader);

			Plane();

		private:
			unsigned int _VAO, _VBO, _EBO;

			void initVAO();

			struct VertexData {
				glm::vec3 pos;
				glm::vec2 uv;
				glm::vec3 normal;
				glm::vec3 tan;
			};
		};

} }