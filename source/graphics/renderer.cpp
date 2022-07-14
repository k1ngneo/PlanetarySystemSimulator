#include "StarSystemSim/graphics/renderer.h"

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace graphics {

	Renderer::Renderer()
		: m_RenderBuffer(0),
		m_VAO(0), m_VBO(0), m_EBO(0),
		blurStr(10)
	{
		m_FrameBuffers[0] = 0;
		m_ScreenTextures[0] = 0;

		setupFramebuffers();
		setupVertexArray();

		m_PostprocessingShader.compileShaders("shaders/postprocessing.shader", true);
		m_PostprocessingShader.linkShaders();

		m_BlurShader.compileShaders("shaders/blur.shader", true);
		m_BlurShader.linkShaders();
	}

	Renderer::~Renderer() {
		if (!m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (!m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (!m_EBO)
			glDeleteBuffers(1, &m_EBO);
		if (!m_FrameBuffers[0])
			glDeleteFramebuffers(s_FBO_COUNT, m_FrameBuffers);
		if (!m_RenderBuffer)
			glDeleteRenderbuffers(1, &m_RenderBuffer);
		if (!m_ScreenTextures[0])
			glDeleteTextures(s_TEXTURE_COUNT, m_ScreenTextures);
	}

	void Renderer::drawFrame() {
		// calculating bloom effect
		glDisable(GL_DEPTH_TEST);
		m_BlurShader.use();
		
		bool horizontal = true;

		glBindVertexArray(m_VAO);
		for (int i = 0; i < this->blurStr * 2; ++i) {
			int inputTextureIndex = 2 + ((i + 1) % 2);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[1 + (i % 2)]);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_BlurShader.setUniform1i("_texture", 0);
			glActiveTexture(GL_TEXTURE0);
			if(i == 0)
				glBindTexture(GL_TEXTURE_2D, m_ScreenTextures[1]);
			else
				glBindTexture(GL_TEXTURE_2D, m_ScreenTextures[inputTextureIndex]);

			m_BlurShader.setUniform1i("_horizontal", horizontal);
			horizontal = !horizontal;

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		}

		m_BlurShader.unuse();

		// drawing to screen
		glViewport(0, 0, App::s_Instance->getWindowWidth(), App::s_Instance->getWindowHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		m_PostprocessingShader.use();
		m_PostprocessingShader.setUniform1i("_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ScreenTextures[0]);

		m_PostprocessingShader.setUniform1i("_bloomTex", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_ScreenTextures[this->blurStr == 0 ? 1 : 3]);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
		m_PostprocessingShader.unuse();

		m_PostprocessingShader.reload();
	}

	void Renderer::bindFramebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[0]);
	}

	void Renderer::unbindFramebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::resize() {
		setupFramebuffers();
	}

	void Renderer::setupFramebuffers() {
		if (m_FrameBuffers[0] != 0) {
			glDeleteFramebuffers(s_FBO_COUNT, m_FrameBuffers);
			for (int i = 0; i < s_FBO_COUNT; ++i) {
				m_FrameBuffers[i] = 0;
			}
		}

		glGenFramebuffers(s_FBO_COUNT, m_FrameBuffers);
		
		// setting up textures
		if (m_ScreenTextures[0] != 0)
			glDeleteTextures(s_TEXTURE_COUNT, m_ScreenTextures);

		glGenTextures(s_TEXTURE_COUNT, m_ScreenTextures);
		for (int textureInd = 0; textureInd < s_TEXTURE_COUNT; ++textureInd) {
			glBindTexture(GL_TEXTURE_2D, m_ScreenTextures[textureInd]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, App::s_Instance->getWindowWidth(), App::s_Instance->getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// main (lighting) framebuffer
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTextures[0], 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_ScreenTextures[1], 0);


			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, colorAttachments);

			if (m_RenderBuffer != 0)
				glDeleteRenderbuffers(1, &m_RenderBuffer);

			glGenRenderbuffers(1, &m_RenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, App::s_Instance->getWindowWidth(), App::s_Instance->getWindowHeight());
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				utils::printError("Failed to create main frame buffer!\n");
			}
		}

		// bloom framebuffers
		{
			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0 };
			
			for (int i = 0; i < 2; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[1 + i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTextures[2 + i], 0);

				glDrawBuffers(1, colorAttachments);

				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
					utils::printError("Failed to create bloom frame buffer!\n");
				}
			}
		}
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

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));

		glBindVertexArray(0);
	}

}