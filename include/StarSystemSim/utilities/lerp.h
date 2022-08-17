#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace utils {
	float lerp(float a, float b, float x);
	glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float x);
	glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float x);
}