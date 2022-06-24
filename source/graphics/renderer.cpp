#include "StarSystemSim/graphics/renderer.h"

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace graphics {

	Renderer::Renderer()
		: m_frameBuffer(0), m_screenTexture(0), m_renderBuffer(0),
		m_vao(0), m_vbo(0), m_ebo(0)
	{
		setupFramebuffer();
		setupVertexArray();

		m_postprocessingShader.compileShaders("shaders/postprocessing.shader", true);
		m_postprocessingShader.linkShaders();
	}

	Renderer::~Renderer() {
		if (!m_vao)
			glDeleteVertexArrays(1, &m_vao);
		if (!m_vbo)
			glDeleteBuffers(1, &m_vbo);
		if (!m_ebo)
			glDeleteBuffers(1, &m_ebo);
		if (!m_frameBuffer)
			glDeleteFramebuffers(1, &m_frameBuffer);
		if (!m_renderBuffer)
			glDeleteRenderbuffers(1, &m_renderBuffer);
		if (!m_screenTexture)
			glDeleteTextures(1, &m_screenTexture);
	}

	void Renderer::drawFrame() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.9f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		m_postprocessingShader.use();
		m_postprocessingShader.setUniform1i("_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenTexture);

		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
		m_postprocessingShader.unuse();

		m_postprocessingShader.reload();
	}

	void Renderer::bindFramebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	}

	void Renderer::unbindFramebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::setupFramebuffer() {
		glGenFramebuffers(1, &m_frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

		glGenTextures(1, &m_screenTexture);
		glBindTexture(GL_TEXTURE_2D, m_screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app::SCR_WIDTH, app::SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screenTexture, 0);

		glGenRenderbuffers(1, &m_renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, app::SCR_WIDTH, app::SCR_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			utils::printError("Failed to create screen frame buffer!\n");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::setupVertexArray() {
		struct VertexData {
			glm::vec2 pos;
			glm::vec2 uv;
		};

		VertexData vertices[4] = {
			{{-1.0f,  1.0f}, {0.0f, 1.0f}}, // top-left
			{{ 1.0f,  1.0f}, {1.0f, 1.0f}}, // top-right
			{{-1.0f, -1.0f}, {0.0f, 0.0f}}, // bottom-left
			{{ 1.0f, -1.0f}, {1.0f, 0.0f}}, // bottom-right
		};

		unsigned int indices[] = {
			0, 1, 2, // first triangle
			1, 3, 2  // second triangle
		};

		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);

		glBindVertexArray(m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));

		glBindVertexArray(0);
	}

}