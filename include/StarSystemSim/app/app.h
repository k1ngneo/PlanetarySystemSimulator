#pragma once

#include "StarSystemSim/graphics/renderer.h"
#include "StarSystemSim/graphics/scene.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/physics/engine.h"
#include "StarSystemSim/utilities/timer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>

#include <cstdint>
#include <vector>

struct App {
	static utils::Timer mainTimer;
	static utils::Timer frameClock;

	static void start(uint32_t width = 1400, uint32_t height = 900);
	static void clear();

	static uint32_t getWindowWidth();
	static uint32_t getWindowHeight();

	static graphics::Planet* addToScene(graphics::Planet& planet);
	static graphics::Star* addToScene(graphics::Star& star);
	static void setSkybox(const graphics::Skybox& skybox);

	static App* s_Instance;
	static GLFWwindow* s_Window;


	graphics::Renderer renderer;
	graphics::RenderMode renderMode;
	physics::Engine physicsEngine;

	graphics::Scene scene;
	graphics::Camera mainCamera;
	std::vector<graphics::Object*> camTargets;


	bool isCursorVisible;
	bool isFirstMouseMovement;
	glm::vec2 mousePos;

	App(uint32_t windowWidth, uint32_t windowHeight);
	~App();


	void resize(uint32_t width, uint32_t height);

private:
	uint16_t m_ScrWidth;
	uint16_t m_ScrHeight;
};

namespace app {
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
}