#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace graphics {

	class Object {
	public:
		glm::mat4 modelMat;

		void translate(const glm::vec3& trans);
		void rotate(float angle, const glm::vec3& axis);
		void scale(const glm::vec3& sc);

		const glm::mat4& updateModelMat();

		Object();

		glm::vec3 getPos();

		inline void resetTransMat() { _transMat = glm::mat4(1.0f); }
		inline void resetRotMat() { _rotMat = glm::mat4(1.0f); }
		inline void resetScaleMat() { _scaleMat = glm::mat4(1.0f); }

	private:
		glm::mat4 _transMat;
		glm::mat4 _rotMat;
		glm::mat4 _scaleMat;
	};

}