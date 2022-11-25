#include "StarSystemSim/graphics/star.h"

namespace graphics {

	Star::Star(const char* name)
		: Planet(name, 5)
	{
		this->type = Type::STAR;

		this->light.ambientColor = glm::vec3(0.1f);
		this->light.diffuseColor = glm::vec3(1.0f);
		this->light.specularColor = glm::vec3(1.0f);

		this->light.attenuation = glm::vec3(1.0f, 0.0001f, 0.00012f);

		this->body.type = physics::Body::Type::STATIC;
	}

	Star::~Star() {
	}

}