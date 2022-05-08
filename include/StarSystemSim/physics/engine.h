#pragma once

#include "StarSystemSim/physics/body.h"
#include "StarSystemSim/utilities/timer.h"

#include <glm/vec3.hpp>
#include <set>

namespace physics {

	const float GRAVITATIONAL_CONSTANT = 0.5f;

	class Body;

	class Engine {
	public:
		Engine();
		~Engine();

		void addBody(Body* body);
		void remBody(Body* body);

		void update();

		float timeMultiplier;

	private:
		std::set<Body*> m_Bodies;
		utils::Timer m_Timer;
		bool m_FirstIteration;

		void applyGravityForce();
		void advanceBodies();
	};

}