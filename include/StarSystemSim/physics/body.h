#pragma once

#include "StarSystemSim/physics/engine.h"

#include <glm/vec3.hpp>

namespace physics {

	class Engine;

	class Body {
	public:
		Body();
		Body(const glm::vec3 pos, float mass = 1.0f);
		~Body();

		void activate(physics::Engine& engine);

		glm::vec3 pos;
		glm::vec3 vel;
		float mass;

		enum class Type {
			STATIC, DYNAMIC
		} type;
	};

}