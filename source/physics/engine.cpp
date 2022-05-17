#include "StarSystemSim/physics/engine.h"

#include <glm/geometric.hpp>
#include <cmath>

namespace physics {

	Engine::Engine()
		: paused(false),
		m_SkipIteration(true)
	{
		this->timeMultiplier = 1.0f;
	}

	Engine::~Engine() {
	}

	void Engine::addBody(Body* body) {
		m_Bodies.insert(body);
	}

	void Engine::remBody(Body* body) {
		m_Bodies.erase(body);
	}

	void Engine::update() {
		m_Timer.measureTime();
		m_Timer.deltaTime *= this->paused ? 0.0f : this->timeMultiplier;

		if (m_SkipIteration) {
			m_SkipIteration = false;
			return;
		}

		applyGravityForce();
		advanceBodies();
	}

	void Engine::skipIteration() {
		m_SkipIteration = true;
	}

	void Engine::applyGravityForce() {
		for (auto iterA = m_Bodies.begin(); iterA != m_Bodies.end(); ++iterA) {
			for (auto iterB = m_Bodies.begin(); iterB != m_Bodies.end(); ++iterB) {
				if (iterA == iterB)
					continue;

				// calculate the gravity force
				// F = m * a = G * (M1 * M2) / (R^2)
				glm::vec3 AtoB = (*iterB)->pos - (*iterA)->pos;
				glm::vec3 AtoBsq = AtoB * AtoB;
				float dist = sqrt(AtoBsq.x + AtoBsq.y + AtoBsq.z);
				float forceMag = GRAVITATIONAL_CONSTANT * ((*iterA)->mass * (*iterB)->mass) / dist;

				glm::vec3 accA = glm::normalize(AtoB) * (forceMag / (*iterA)->mass);
				glm::vec3 accB = glm::normalize(-AtoB) * (forceMag / (*iterB)->mass);

				(*iterA)->vel += accA * m_Timer.deltaTime;
				(*iterB)->vel += accB * m_Timer.deltaTime;
			}
		}
	}

	void Engine::advanceBodies() {
		for (auto iter = m_Bodies.begin(); iter != m_Bodies.end(); ++iter) {
			(*iter)->pos = (*iter)->pos + (*iter)->vel * m_Timer.deltaTime;
		}
	}

}