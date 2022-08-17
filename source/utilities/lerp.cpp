#include "StarSystemSim/utilities/lerp.h"

namespace utils {
	float lerp(float a, float b, float x) {
		return a + x * (b - a);
	}

	glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float x) {
		return a + x * (b - a);
	}

	glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float x) {
		return a + x * (b - a);
	}
}