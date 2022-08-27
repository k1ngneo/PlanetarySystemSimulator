#include "StarSystemSim/physics/body.h"
#include "StarSystemSim/physics/engine.h"

namespace physics {

	Body::Body()
		: pos(0.0f, 0.0f, 0.0f), vel(0.0f, 0.0f, 0.0f), mass(1.0f),
		type(Type::DYNAMIC)
	{
	}

	Body::Body(const glm::vec3 pos, float mass) {
		this->pos = pos;
		this->vel = { 0.0f, 0.0f, 0.0f };
		this->mass = mass;
		this->type = Type::DYNAMIC;
	}

	const Body& Body::operator=(const Body& otherBody) {
		this->pos = otherBody.pos;
		this->vel = otherBody.vel;
		this->mass = otherBody.mass;
		this->type = otherBody.type;

		return *this;
	}

	Body::~Body() {
	}

}