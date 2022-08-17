#pragma once

#include <StarSystemSim/graphics/camera.h>
#include <StarSystemSim/utilities/timer.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>

#include <cstdint>
#include <vector>

class graphics::Object;

struct App {
	static utils::Timer mainTimer;
	static utils::Timer frameClock;

	static void start(uint32_t width = 1400, uint32_t height = 900);
	static void clear();

	static uint32_t getWindowWidth();
	static uint32_t getWindowHeight();

	static App* s_Instance;

	GLFWwindow* window;

	graphics::Camera mainCamera;
	std::vector<graphics::Object*> camTargets;


	bool isCursorVisible;
	bool isFirstMouseMovement;
	glm::vec2 mousePos;

	App(uint32_t windowWidth, uint32_t windowHeight);
	~App();


	void resize(uint32_t width, uint32_t height);

private:
	uint32_t m_ScrWidth;
	uint32_t m_ScrHeight;
};

namespace app {
	void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
}