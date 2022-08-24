#pragma once

#include "StarSystemSim/physics/body.h"
#include "StarSystemSim/utilities/timer.h"

#include <glm/vec3.hpp>
#include <memory>
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

		void getPredictedPos(std::vector<glm::vec3>& pos);

		bool paused;
		float timeMultiplier;

	private:
		std::set<Body*> m_Bodies;
		std::set<Body*> m_BodiesDynamic;
		std::set<Body*> m_BodiesStatic;

		std::vector<std::vector<Body>> m_PosPrediction;

		utils::Timer m_Timer;
		bool m_SkipIteration;

		void applyGravityForce();
		void calcGravityVelChange(Body& body1, Body& body2);
		void advanceBodies();
		void calcFuturePos(uint16_t steps, float timeOffset);
	};

}