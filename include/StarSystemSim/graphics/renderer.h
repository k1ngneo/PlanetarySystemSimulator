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

	private:
		unsigned int m_frameBuffer;
		unsigned int m_screenTexture;
		unsigned int m_renderBuffer;

		unsigned int m_vao, m_vbo, m_ebo;

		Shader m_postprocessingShader;

		void setupFramebuffer();
		void setupVertexArray();
	};

}