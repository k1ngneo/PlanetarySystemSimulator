#include "StarSystemSim/physics/body.h"
#include "StarSystemSim/physics/engine.h"

namespace physics {

	Body::Body()
		: pos(0.0f, 0.0f, 0.0f), vel(0.0f, 0.0f, 0.0f), mass(1.0f)
	{
	}

	Body::Body(const glm::vec3 pos, float mass) {
		this->pos = pos;
		this->vel = { 0.0f, 0.0f, 0.0f };
		this->mass = mass;
	}

	Body::~Body() {
	}

	void Body::activate(physics::Engine& engine) {
		engine.addBody(this);
	}

}