#include "StarSystemSim/physics/engine.h"

#include <glm/geometric.hpp>
#include <cmath>

namespace physics {

	Engine::Engine()
		: paused(true),
		m_SkipIteration(true)
	{
		this->timeMultiplier = 1.0f;
	}

	Engine::~Engine() {
		m_Bodies.clear();
		m_BodiesDynamic.clear();
		m_BodiesStatic.clear();
	}

	void Engine::addBody(Body* body) {
		m_Bodies.insert(body);
		if (body->type == Body::Type::DYNAMIC)
			m_BodiesDynamic.insert(body);
		else if (body->type == Body::Type::STATIC)
			m_BodiesStatic.insert(body);
	}

	void Engine::remBody(Body* body) {
		m_Bodies.erase(body);
		m_BodiesDynamic.erase(body);
		m_BodiesStatic.erase(body);
	}

	void Engine::update() {
		m_Timer.measureTime();
		m_Timer.deltaTime *= this->paused ? 0.0f : this->timeMultiplier;

		m_Timer.deltaTime = std::min(m_Timer.deltaTime, MAX_DELTA_TIME);

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
			for (auto iterB = iterA; iterB != m_Bodies.end(); ++iterB) {
				if (iterA == iterB)
					continue;

				// calculate the gravity force
				// F = m * a = G * (M1 * M2) / (R^2)
				glm::vec3 AtoB = (*iterB)->pos - (*iterA)->pos;
				glm::vec3 AtoBsq = AtoB * AtoB;
				float dist = sqrt(AtoBsq.x + AtoBsq.y + AtoBsq.z);
				float forceMag = GRAVITATIONAL_CONSTANT * ((*iterA)->mass * (*iterB)->mass) / (dist*dist);

				glm::vec3 accA = glm::normalize(AtoB) * (forceMag / (*iterA)->mass);
				glm::vec3 accB = glm::normalize(-AtoB) * (forceMag / (*iterB)->mass);

				(*iterA)->vel += accA * m_Timer.deltaTime;
				(*iterB)->vel += accB * m_Timer.deltaTime;
			}
		}
	}

	void Engine::advanceBodies() {
		for (auto iter = m_Bodies.begin(); iter != m_Bodies.end(); ++iter) {
			if ((*iter)->type == Body::Type::DYNAMIC) {
				(*iter)->pos = (*iter)->pos + (*iter)->vel * m_Timer.deltaTime;
			}
		}
	}

}