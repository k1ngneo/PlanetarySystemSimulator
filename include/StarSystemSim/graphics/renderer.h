#pragma once

#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/scene.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/graphics/skybox.h"
#include "StarSystemSim/graphics/object.h"
#include "StarSystemSim/graphics/star.h"
#include "StarSystemSim/graphics/render_mode.h"

#include <glad/glad.h>

#include <vector>

namespace graphics {

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void drawFrame(uint32_t renderMode = (uint32_t)RenderMode::FACES);

		void bindScene(Scene* scene);

		void resize();
		void resize(uint16_t width, uint16_t height);

		uint8_t MSAA_samples;
		int blurStr;

		int currentBloomTexture;
		inline size_t getBloomTextureCount() { return m_BloomTextureCount; }

		std::vector<glm::vec3>* lines;

	private:
		Scene* m_CurrentScene;


		// Framebuffers:
		// Main rendering multisampled framebuffer
		unsigned int m_MSFramebuffer;
		// MSAA intermediate framebuffer
		unsigned int m_IntermediateMSFramebuffer;
		// HDR transition framebuffer
		unsigned int m_HDRFramebuffer;
		// switching bloom framebuffers
		unsigned int* m_BloomFramebuffers;

		// Textures:
		// Main texture
		unsigned int m_MainTexture;
		// HDR texture
		unsigned int m_HDRTexture;
		// buffers for bloom calculations
		size_t m_BloomTextureCount;
		unsigned int* m_BloomTextures;

		// multisampled render buffers
		unsigned int m_RenderBuffers[3];

		unsigned int m_VAO, m_VBO, m_EBO;

		
		Shader m_PostprocessingShader;
		Shader m_BlurShader;
		Shader m_MixShader;
		Shader m_SkyboxShader;
		Shader m_CelestialShader;
		Shader m_StarShader;
		Shader m_LineShader;

		void setupFramebuffers();
		void setupFramebuffers(uint16_t width, uint16_t height);
		void setupVertexArray();
		void setupShaders();
	};

}