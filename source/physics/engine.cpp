#include "StarSystemSim/physics/engine.h"

#include "StarSystemSim/physics/body.h"

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
		}
		else {
			applyGravityForce();
			advanceBodies();
		}

		calcFuturePos(100, 0.02f);
	}

	void Engine::skipIteration() {
		m_SkipIteration = true;
	}

	void Engine::getPredictedPos(std::vector<glm::vec3>& positions) {
		positions.clear();
		
		for (size_t iter1 = 0; iter1 < m_PosPrediction.size(); ++iter1) {
			for (size_t iter2 = 0; iter2 < m_PosPrediction[iter1].size() - 1; ++iter2) {
				positions.push_back(m_PosPrediction[iter1][iter2 + 0].pos);
				positions.push_back(m_PosPrediction[iter1][iter2 + 1].pos);
			}
		}
	}

	void Engine::applyGravityForce() {
		for (auto iterA = m_Bodies.begin(); iterA != m_Bodies.end(); ++iterA) {
			for (auto iterB = iterA; iterB != m_Bodies.end(); ++iterB) {
				if (iterA == iterB)
					continue;

				calcGravityVelChange(**iterA, **iterB);
			}
		}
	}

	void Engine::calcGravityVelChange(Body& bodyA, Body& bodyB) {
		// calculating the gravity force
		// F = m * a = G * (M1 * M2) / (R^2)
		glm::vec3 AtoB = bodyB.pos - bodyA.pos;
		glm::vec3 AtoBsq = AtoB * AtoB;
		float dist = sqrt(AtoBsq.x + AtoBsq.y + AtoBsq.z);
		float forceMag = GRAVITATIONAL_CONSTANT * (bodyA.mass * bodyB.mass) / (dist*dist);

		glm::vec3 accA = glm::normalize(AtoB) * (forceMag / bodyA.mass);
		glm::vec3 accB = glm::normalize(-AtoB) * (forceMag / bodyB.mass);

		bodyA.vel += accA * m_Timer.deltaTime;
		bodyB.vel += accB * m_Timer.deltaTime;
	}

	void Engine::advanceBodies() {
		for (auto iter = m_Bodies.begin(); iter != m_Bodies.end(); ++iter) {
			if ((*iter)->type == Body::Type::DYNAMIC) {
				(*iter)->pos = (*iter)->pos + (*iter)->vel * m_Timer.deltaTime;
			}
		}
	}

	void Engine::calcFuturePos(uint16_t steps, float timeOffset) {
		if (steps < 1)
			return;
		
		m_PosPrediction = std::move(std::vector<std::vector<Body>>(m_Bodies.size()));

		size_t i = 0;
		for (auto iter = m_Bodies.begin(); iter != m_Bodies.end(); ++iter) {
			m_PosPrediction[i].push_back(**iter);
			m_PosPrediction[i].resize(steps);
			i += 1;
		}

		for (uint16_t step = 1; step < steps; ++step) {

			for (size_t iter1 = 0; iter1 < m_PosPrediction.size(); ++iter1) {
				for (size_t iter2 = iter1; iter2 < m_PosPrediction.size(); ++iter2) {
					if (iter1 == iter2)
						continue;

					calcGravityVelChange(m_PosPrediction[iter1][step], m_PosPrediction[iter2][step]);
				}
			}

			for (size_t iter = 0; iter < m_PosPrediction[step].size(); ++iter) {
				if (m_PosPrediction[iter][step].type == Body::Type::DYNAMIC) {
					m_PosPrediction[iter][step].pos += m_PosPrediction[iter][step-1].vel * timeOffset;
				}
			}
		}
	}

}