#pragma once

#include "StarSystemSim/graphics/shader.h"
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

		void resize();

		void addStar(Star* star);
		void addObject(Object* obj);

		uint8_t MSAA_samples;
		int blurStr;

	private:
		Skybox m_Skybox;
		std::vector<Star*> m_Stars;
		std::vector<Object*> m_Objects;


		// Framebuffers:
		// Main rendering multisampled framebuffer
		unsigned int m_MSFramebuffer;
		// MSAA intermediate framebuffer
		unsigned int m_IntermediateMSFramebuffer;
		// HDR transition framebuffer
		unsigned int m_HDRFramebuffer;
		// switching blur framebuffers
		unsigned int m_BlurFramebuffers[2];

		// Textures:
		// Main texture
		unsigned int m_MainTexture;
		// HDR texture
		unsigned int m_HDRTexture;
		// 2. and 3. buffers for blur calculations
		static const size_t s_TEXTURE_COUNT = 2;
		unsigned int m_BlurTextures[s_TEXTURE_COUNT];

		// multisampled render buffers
		unsigned int m_RenderBuffers[3];

		unsigned int m_VAO, m_VBO, m_EBO;

		
		Shader m_PostprocessingShader;
		Shader m_BlurShader;
		Shader m_SkyboxShader;
		Shader m_CelestialShader;
		Shader m_StarShader;

		void setupFramebuffers();
		void setupVertexArray();
		void setupShaders();
	};

}