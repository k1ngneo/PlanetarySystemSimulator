#pragma once

#include "StarSystemSim/graphics/mesh.h"
#include "StarSystemSim/graphics/planet.h"

namespace graphics {

	class Star : public Planet {
	public:
		Star(const char* name);
		~Star();


		struct Light {
			glm::vec3 ambientColor;
			glm::vec3 diffuseColor;
			glm::vec3 specularColor;

			glm::vec3 attenuation;
		} light;
	};

}