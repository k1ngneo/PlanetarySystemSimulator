#include "StarSystemSim/graphics/star.h"

namespace graphics {

	Star::Star(const char* name)
		: Planet(name)
	{
		this->ambientColor = glm::vec3(0.1f);
		this->diffuseColor = glm::vec3(1.0f);
		this->specularColor = glm::vec3(1.0f);

		this->attenuation = glm::vec3(1.0f, 0.0001f, 0.00012f);

		this->body.type = physics::Body::Type::STATIC;
	}

	Star::~Star() {
	}

}