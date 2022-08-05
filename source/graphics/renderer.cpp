#include "StarSystemSim/graphics/renderer.h"

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/graphics/render_mode.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace graphics {

	Renderer::Renderer()
		: m_VAO(0), m_VBO(0), m_EBO(0),
		MSAA_samples(8), blurStr(5),
		m_MSFramebuffer(0), m_HDRFramebuffer(0), m_IntermediateMSFramebuffer(0),
		m_MainTexture(0), m_HDRTexture(0)
	{
		m_BlurFramebuffers[0] = 0;
		m_BlurFramebuffers[1] = 0;

		for (int i = 0; i < 3; ++i) {
			m_RenderBuffers[i] = 0;
		}

		for (int i = 0; i < s_TEXTURE_COUNT; ++i) {
			m_BlurTextures[i] = 0;
		}

		setupFramebuffers();
		setupVertexArray();
		setupShaders();

		m_Skybox.loadTexture("models/skybox/");

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	Renderer::~Renderer() {
		if (!m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (!m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (!m_EBO)
			glDeleteBuffers(1, &m_EBO);
		if (!m_MSFramebuffer)
			glDeleteFramebuffers(1, &m_MSFramebuffer);
		if (!m_IntermediateMSFramebuffer)
			glDeleteFramebuffers(1, &m_IntermediateMSFramebuffer);
		if (!m_HDRFramebuffer)
			glDeleteFramebuffers(1, &m_HDRFramebuffer);
		if (!m_BlurFramebuffers[0])
			glDeleteFramebuffers(1, &m_BlurFramebuffers[0]);
		if (!m_BlurFramebuffers[1])
			glDeleteFramebuffers(1, &m_BlurFramebuffers[1]);
		if (!m_RenderBuffers[0])
			glDeleteRenderbuffers(3, m_RenderBuffers);
		if (!m_MainTexture)
			glDeleteTextures(1, &m_MainTexture);
		if (!m_HDRTexture)
			glDeleteTextures(1, &m_HDRTexture);
		if (!m_BlurTextures[0])
			glDeleteTextures(s_TEXTURE_COUNT, m_BlurTextures);
	}

	void Renderer::drawFrame(uint32_t renderMode) {

		// rendering scene's objects
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_MSFramebuffer);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			// drawing skybox
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				m_SkyboxShader.use();
				m_SkyboxShader.setUniformMat4("_projMat", App::s_Instance->mainCamera.projMatrix);
				m_SkyboxShader.setUniformMat4("_viewMat", glm::mat4(glm::mat3(App::s_Instance->mainCamera.viewMatrix)));
				m_SkyboxShader.unuse();

				m_Skybox.draw(m_SkyboxShader);
			}

			switch ((RenderMode)renderMode) {
				case RenderMode::FACES:
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					break;
				case RenderMode::LINES:
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					break;
				case RenderMode::POINTS:
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
					break;
				default:
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			// drawing stars
			{
				m_StarShader.use();
				m_StarShader.setUniformMat4("_projMat", App::s_Instance->mainCamera.projMatrix);
				m_StarShader.setUniformMat4("_viewMat", App::s_Instance->mainCamera.viewMatrix);
				m_StarShader.unuse();

				for (Star* star : m_Stars) {
					star->draw(m_StarShader);
				}
			}

			// drawing planets
			{
				m_CelestialShader.use();
				m_CelestialShader.setUniformMat4("_projMat", App::s_Instance->mainCamera.projMatrix);
				m_CelestialShader.setUniformMat4("_viewMat", App::s_Instance->mainCamera.viewMatrix);

				m_CelestialShader.setUniform3f("_viewPos", App::s_Instance->mainCamera.pos);

				m_CelestialShader.setUniform3f("_light1.pos", m_Stars[0]->getPos());
				m_CelestialShader.setUniform3f("_light1.amb", m_Stars[0]->ambientColor);
				m_CelestialShader.setUniform3f("_light1.diff", m_Stars[0]->diffuseColor);
				m_CelestialShader.setUniform3f("_light1.spec", m_Stars[0]->specularColor);
				m_CelestialShader.setUniform3f("_light1.attenuation", m_Stars[0]->attenuation);

				m_CelestialShader.setUniform1f("_time", App::mainTimer.lastTime);
				m_CelestialShader.unuse();

				for (Object* object : m_Objects) {
					object->draw(m_CelestialShader, GL_TRIANGLES);
				}
			}
		}
		
		// blitting from a multisampled frame buffer
		{
			glDisable(GL_DEPTH_TEST);
			int32_t width = App::getWindowWidth(), height = App::getWindowHeight();

			// blitting main rendered image
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSFramebuffer);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_IntermediateMSFramebuffer);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSFramebuffer);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_IntermediateMSFramebuffer);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		glDisable(GL_DEPTH_TEST);

		// calculating bloom effect
		{
			m_BlurShader.use();
		
			bool horizontal = true;
		
			glBindVertexArray(m_VAO);
			for (int i = 0; i < this->blurStr * 2; ++i) {
				int inputTextureIndex = ((i + 1) % 2);
				glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFramebuffers[i % 2]);
				//glClearColor(0.0, 0.0, 0.0, 1.0);
				//glClear(GL_COLOR_BUFFER_BIT);
		
				m_BlurShader.setUniform1i("_texture", 0);
				glActiveTexture(GL_TEXTURE0);
				if(i == 0)
					glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
				else
					glBindTexture(GL_TEXTURE_2D, m_BlurTextures[inputTextureIndex]);
		
				m_BlurShader.setUniform1i("_horizontal", horizontal);
				horizontal = !horizontal;
		
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
			}

			m_BlurShader.unuse();
		}
		

		// drawing to screen
		{
			glViewport(0, 0, App::getWindowWidth(), App::getWindowHeight());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			
			m_PostprocessingShader.use();
			m_PostprocessingShader.setUniform1i("_texture", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_MainTexture);
			
			m_PostprocessingShader.setUniform1i("_bloomTex", 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_BlurTextures[0]);
			
			glBindVertexArray(m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
			glBindVertexArray(0);
			m_PostprocessingShader.unuse();
		}

		m_PostprocessingShader.reload();
		m_CelestialShader.reload();
		m_BlurShader.reload();
	}

	void Renderer::resize() {
		setupFramebuffers();
	}

	void Renderer::addStar(Star* star) {
		m_Stars.push_back(star);
	}

	void Renderer::addObject(Object* obj) {
		m_Objects.push_back(obj);
	}

	void Renderer::setupFramebuffers() {
		// reloading framebuffers if necessary
		{
			if (m_MSFramebuffer)
				glDeleteFramebuffers(1, &m_MSFramebuffer);
			glGenFramebuffers(1, &m_MSFramebuffer);

			if (m_HDRFramebuffer)
				glDeleteFramebuffers(1, &m_HDRFramebuffer);
			glGenFramebuffers(1, &m_HDRFramebuffer);

			if (m_BlurFramebuffers[0])
				glDeleteFramebuffers(2, m_BlurFramebuffers);
			glGenFramebuffers(2, m_BlurFramebuffers);

			if (m_IntermediateMSFramebuffer)
				glDeleteFramebuffers(1, &m_IntermediateMSFramebuffer);
			glGenFramebuffers(1, &m_IntermediateMSFramebuffer);
		}

		// setting up textures
		{
			// Main Texture
			{
				if (m_MainTexture)
					glDeleteTextures(1, &m_MainTexture);
				glGenTextures(1, &m_MainTexture);

				glBindTexture(GL_TEXTURE_2D, m_MainTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, App::getWindowWidth(), App::getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			// HDR Texture
			{
				if (m_HDRTexture)
					glDeleteTextures(1, &m_HDRTexture);
				glGenTextures(1, &m_HDRTexture);

				glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, App::getWindowWidth(), App::getWindowHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			// switching blur buffer textures
			{
				if (m_BlurTextures[0] != 0)
					glDeleteTextures(s_TEXTURE_COUNT, m_BlurTextures);

				glGenTextures(s_TEXTURE_COUNT, m_BlurTextures);
				for (int textureInd = 0; textureInd < s_TEXTURE_COUNT; ++textureInd) {
					glBindTexture(GL_TEXTURE_2D, m_BlurTextures[textureInd]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, App::getWindowWidth(), App::getWindowHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}

		// main (lighting) framebuffer
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_MSFramebuffer);
			
			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

			if (m_RenderBuffers[0] != 0)
				glDeleteRenderbuffers(3, m_RenderBuffers);

			glGenRenderbuffers(3, m_RenderBuffers);

			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffers[0]);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_RGB, App::getWindowWidth(), App::getWindowHeight());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_RenderBuffers[0]);

			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffers[1]);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_RGBA32F, App::getWindowWidth(), App::getWindowHeight());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, m_RenderBuffers[1]);

			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffers[2]);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_DEPTH24_STENCIL8, App::getWindowWidth(), App::getWindowHeight());
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffers[2]);

			glDrawBuffers(2, colorAttachments);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				utils::printError("Failed to create main frame buffer!");
			}
		}

		// bloom framebuffers
		{
			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0 };
			
			for (int i = 0; i < 2; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFramebuffers[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BlurTextures[i], 0);

				glDrawBuffers(1, colorAttachments);

				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
					utils::printError("Failed to create bloom frame buffer number %d!", i);
				}
			}
		}

		// multisample intermediate framebuffer
		{
			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			
			glBindFramebuffer(GL_FRAMEBUFFER, m_IntermediateMSFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MainTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_HDRTexture, 0);


			glDrawBuffers(2, colorAttachments);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				utils::printError("Failed to create MSAA intermediate frame buffer!");
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
	void Renderer::setupShaders() {
		m_PostprocessingShader.compileShaders("shaders/postprocessing.shader", true);
		m_PostprocessingShader.linkShaders();

		m_BlurShader.compileShaders("shaders/blur.shader", true);
		m_BlurShader.linkShaders();

		m_SkyboxShader.compileShaders("shaders/skybox.shader", true);
		m_SkyboxShader.linkShaders();

		m_CelestialShader.compileShaders("shaders/celestial.shader", true);
		m_CelestialShader.linkShaders();
		
		m_StarShader.compileShaders("shaders/star.shader", true);
		m_StarShader.linkShaders();
	}
}