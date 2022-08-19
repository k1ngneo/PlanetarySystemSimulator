#pragma once

#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/planet.h"
#include "StarSystemSim/graphics/star.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/graphics/skybox.h"

#include <vector>

namespace graphics {

	struct Scene {
		Skybox skybox;
		std::vector<Star*> stars;
		std::vector<Planet*> planets;

		~Scene();
	};

}