#include "StarSystemSim/graphics/object.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace graphics {

	void Object::translate(const glm::vec3& trans) {
		_transMat = glm::translate(_transMat, trans);
	}

	void Object::rotate(float angle, const glm::vec3& axis) {
		angle = glm::radians(angle);
		glm::vec3 normAxis = glm::normalize(axis);

		glm::quat quaternion = glm::angleAxis(angle, normAxis);
		_rotMat = glm::toMat4(quaternion) * _rotMat;
	}

	void Object::scale(const glm::vec3& sc) {
		_scaleMat = glm::scale(_scaleMat, sc);
	}

	const glm::mat4& Object::updateModelMat() {
		this->modelMat = _transMat * _rotMat * _scaleMat;
		return this->modelMat;
	}

	Object::Object(Type objType)
	: type(objType)
	{
		this->modelMat = glm::mat4(1.0f);
		_transMat = glm::mat4(1.0f);
		_rotMat = glm::mat4(1.0f);
		_scaleMat = glm::mat4(1.0f);
	}

	glm::vec3 Object::getPos() {
		return _transMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	float Object::getMousePickRadius() {
		glm::vec4 scale = _scaleMat * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		return (scale.x + scale.y + scale.z) * 0.33f;
	}

}