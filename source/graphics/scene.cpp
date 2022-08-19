#include "StarSystemSim/graphics/scene.h"

namespace graphics {

	Scene::~Scene() {
		for (size_t iter = 0; iter < stars.size(); ++iter) {
			delete stars[iter];
		}
		stars.clear();

		for (size_t iter = 0; iter < planets.size(); ++iter) {
			delete planets[iter];
		}
		planets.clear();
	}

}