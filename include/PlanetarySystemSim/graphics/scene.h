#pragma once

#include "PlanetarySystemSim/graphics/object.h"
#include "PlanetarySystemSim/graphics/planet.h"
#include "PlanetarySystemSim/graphics/star.h"
#include "PlanetarySystemSim/graphics/camera.h"
#include "PlanetarySystemSim/graphics/skybox.h"

#include <vector>

namespace graphics {

	struct Scene {
		Skybox skybox;
		std::vector<Star*> stars;
		std::vector<Planet*> planets;

		~Scene();
	};

}