#pragma once

#include <glm/vec3.hpp>

namespace graphics {

	struct Material {
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};

}