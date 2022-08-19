#pragma once

#include "StarSystemSim/physics/body.h"
#include "StarSystemSim/utilities/timer.h"

#include <glm/vec3.hpp>
#include <vector>
#include <set>

namespace physics {

	const float GRAVITATIONAL_CONSTANT = 0.5f;
	const float MAX_DELTA_TIME = 0.034f;

	class Body;

	class Engine {
	public:
		Engine();
		~Engine();

		void addBody(Body* body);
		void remBody(Body* body);

		void update();

		void skipIteration();

		bool paused;
		float timeMultiplier;

	private:
		std::set<Body*> m_Bodies;
		std::set<Body*> m_BodiesDynamic;
		std::set<Body*> m_BodiesStatic;

		utils::Timer m_Timer;
		bool m_SkipIteration;

		void applyGravityForce();
		void advanceBodies();
	};

}