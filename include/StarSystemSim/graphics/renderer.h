#pragma once

#include "StarSystemSim/graphics/shader.h"

namespace graphics {

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void drawFrame();

		void bindFramebuffer();
		void unbindFramebuffer();

		void resize();

		int blurStr;

	private:
		static const size_t s_FBO_COUNT = 3;
		unsigned int m_FrameBuffers[s_FBO_COUNT];
		static const size_t s_TEXTURE_COUNT = 4;
		unsigned int m_ScreenTextures[s_TEXTURE_COUNT];
		unsigned int m_RenderBuffer;

		unsigned int m_VAO, m_VBO, m_EBO;

		Shader m_PostprocessingShader;
		Shader m_BlurShader;

		void setupFramebuffers();
		void setupVertexArray();
	};

}