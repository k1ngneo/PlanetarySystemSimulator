#include "StarSystemSim/utilities/timer.h"

#include <GLFW/glfw3.h>

namespace utils {

	float Timer::measureTime() {
		double now = glfwGetTime();
		deltaTime = (float)(now - lastTime);
		lastTime = now;

		return now;
	}

}