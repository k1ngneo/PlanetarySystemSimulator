#include "StarSystemSim/graphics/renderer.h"

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/graphics/render_mode.h"
#include "StarSystemSim/utilities/error.h"

#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace graphics {

	float Renderer::tessLevel = 1.0f;

	Renderer::Renderer()
		: m_VAO(0), m_VBO(0), m_EBO(0),
		MSAA_samples(8), blurStr(5),
		m_CurrentScene(nullptr),
		m_MSFramebuffer(0), m_HDRFramebuffer(0), m_IntermediateMSFramebuffer(0), m_BloomFramebuffers(nullptr),
		m_MainTexture(0), m_HDRTexture(0), m_BloomTextures(nullptr),
		bloomEnabled(false)
	{
		this->currentBloomTexture = 1;

		for (int i = 0; i < 3; ++i) {
			m_RenderBuffers[i] = 0;
		}

		setupFramebuffers();
		setupVertexArray();
		setupShaders();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		// tessellation patch vertices
		glPatchParameteri(GL_PATCH_VERTICES, 3);
	}

	Renderer::~Renderer() {
		if (m_VAO)
			glDeleteVertexArrays(1, &m_VAO);
		if (m_VBO)
			glDeleteBuffers(1, &m_VBO);
		if (m_EBO)
			glDeleteBuffers(1, &m_EBO);
		if (m_MSFramebuffer)
			glDeleteFramebuffers(1, &m_MSFramebuffer);
		if (m_IntermediateMSFramebuffer)
			glDeleteFramebuffers(1, &m_IntermediateMSFramebuffer);
		if (m_HDRFramebuffer)
			glDeleteFramebuffers(1, &m_HDRFramebuffer);
		if (m_BloomFramebuffers) {
			glDeleteFramebuffers(m_BloomTextureCount, m_BloomFramebuffers);
			delete[] m_BloomFramebuffers;
		}
		if (m_RenderBuffers[0])
			glDeleteRenderbuffers(3, m_RenderBuffers);
		if (m_MainTexture)
			glDeleteTextures(1, &m_MainTexture);
		if (m_HDRTexture)
			glDeleteTextures(1, &m_HDRTexture);
		if (m_BloomTextures) {
			glDeleteTextures(m_BloomTextureCount, m_BloomTextures);
			delete[] m_BloomTextures;
		}
	}

	void Renderer::drawFrame(uint32_t renderMode) {

		// rendering scene's objects
		if(m_CurrentScene != nullptr) {
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

				m_CurrentScene->skybox.draw(m_SkyboxShader);
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
				m_StarShader.setUniform1ui("_bloomEnabled", this->bloomEnabled);

				for (Star* star : m_CurrentScene->stars) {
					m_StarShader.setUniform3f("_diffColor", star->light.diffuseColor);
					star->draw(m_StarShader);
				}

				m_StarShader.unuse();
			}

			// drawing planets
			{
				m_CelestialShader.use();
				m_CelestialShader.setUniformMat4("_projMat", App::s_Instance->mainCamera.projMatrix);
				m_CelestialShader.setUniformMat4("_viewMat", App::s_Instance->mainCamera.viewMatrix);

				m_CelestialShader.setUniform3f("_viewPos", App::s_Instance->mainCamera.pos);

				m_CelestialShader.setUniform3f("_light.tanPos", m_CurrentScene->stars[0]->getPos());
				m_CelestialShader.setUniform3f("_light.amb", m_CurrentScene->stars[0]->light.ambientColor);
				m_CelestialShader.setUniform3f("_light.diff", m_CurrentScene->stars[0]->light.diffuseColor);
				m_CelestialShader.setUniform3f("_light.spec", m_CurrentScene->stars[0]->light.specularColor);
				m_CelestialShader.setUniform3f("_light.att", m_CurrentScene->stars[0]->light.attenuation);

				m_CelestialShader.setUniform1f("_time", App::mainTimer.lastTime);

				m_CelestialShader.unuse();

				for (Object* object : m_CurrentScene->planets) {
					object->draw(m_CelestialShader, GL_PATCHES);
				}
			}

			// drawing lines
			{
				GLuint vao, vbo;
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
			
				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lines->size(), (void*)lines->data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);
			
				m_LineShader.use();
				m_LineShader.setUniformMat4("_projMat", App::s_Instance->mainCamera.projMatrix);
				m_LineShader.setUniformMat4("_viewMat", App::s_Instance->mainCamera.viewMatrix);
			
				glDrawArrays(GL_LINES, 0, lines->size() / 2);
			
				m_LineShader.unuse();
			
				glBindVertexArray(0);
			
				glDeleteBuffers(1, &vbo);
				glDeleteVertexArrays(1, &vao);
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
		if(bloomEnabled)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// downscaling
			m_BlurShader.use();
			m_BlurShader.setUniform1i("_texture", 0);
			bool horizontal = true;

			int32_t texWidth = App::getWindowWidth(), texHeight = App::getWindowHeight();

			glBindVertexArray(m_VAO);

			for (int texIter = 0; texIter < m_BloomTextureCount; ++texIter) {
				glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFramebuffers[texIter]);
				glViewport(0, 0, texWidth, texHeight);

				if (texIter % 2) {
					texWidth /= 2;
					texHeight /= 2;
				}

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texIter > 0 ? m_BloomTextures[texIter - 1] : m_HDRTexture);

				m_BlurShader.setUniform1i("_horizontal", horizontal);
				horizontal = !horizontal;

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
			}

			m_BlurShader.unuse();

			// upscaling
			m_MixShader.use();
			m_MixShader.setUniform1i("_texture1", 0);
			m_MixShader.setUniform1i("_texture2", 1);

			glBindVertexArray(m_VAO);

			glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFramebuffers[m_BloomTextureCount - 4]);

			GLint width, height;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_BloomTextures[m_BloomTextureCount - 4]);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
			glViewport(0, 0, texWidth, texHeight);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_BloomTextures[m_BloomTextureCount - 1]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_BloomTextures[m_BloomTextureCount - 3]);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			for (int texIter = m_BloomTextureCount-4; texIter >= 2; texIter -= 2) {
				glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFramebuffers[texIter - 2]);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_BloomTextures[texIter - 2]);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
				glViewport(0, 0, texWidth, texHeight);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_BloomTextures[texIter - 0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_BloomTextures[texIter - 1]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			}
			m_MixShader.unuse();
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
			glBindTexture(GL_TEXTURE_2D, m_BloomTextures[1]);
			
			m_PostprocessingShader.setUniform1ui("_bloomEnabled", this->bloomEnabled);

			glBindVertexArray(m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
			glBindVertexArray(0);
			m_PostprocessingShader.unuse();
		}

		//m_PostprocessingShader.reload();
		m_CelestialShader.reload();
		//m_BlurShader.reload();
		//m_StarShader.reload();
	}

	void Renderer::bindScene(Scene* scene) {
		m_CurrentScene = scene;
	}

	void Renderer::resize() {
		setupFramebuffers();
	}
	void Renderer::resize(uint16_t width, uint16_t height) {
		setupFramebuffers(width, height);
	}

	void Renderer::setupFramebuffers() {
		if (App::s_Instance)
			setupFramebuffers(App::getWindowWidth(), App::getWindowHeight());
	}
	void Renderer::setupFramebuffers(uint16_t scrWidth, uint16_t scrHeight) {
		// reloading framebuffers if necessary
		{
			if (m_MSFramebuffer)
				glDeleteFramebuffers(1, &m_MSFramebuffer);
			glGenFramebuffers(1, &m_MSFramebuffer);

			if (m_HDRFramebuffer)
				glDeleteFramebuffers(1, &m_HDRFramebuffer);
			glGenFramebuffers(1, &m_HDRFramebuffer);

			// bloom framebuffers are dealt with just before setting up

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
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scrWidth, scrHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
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
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, scrWidth, scrHeight, 0, GL_RGB, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			// bloom buffer textures
			{
				// deleting old texture buffers and framebuffers if they already exist
				if (m_BloomTextures != nullptr) {
					glDeleteTextures(m_BloomTextureCount, m_BloomTextures);
					delete[] m_BloomTextures;
					m_BloomTextures = nullptr;

					if (m_BloomFramebuffers != nullptr) {
						glDeleteFramebuffers(m_BloomTextureCount, m_BloomFramebuffers);
						delete[] m_BloomFramebuffers;
						m_BloomFramebuffers = nullptr;
					}
				}

				// calculating number of textures
				{
					m_BloomTextureCount = 0;
					size_t width = scrWidth, height = scrHeight;
					while (width > 10 && height > 10) {
						width /= 2;
						height /= 2;
						m_BloomTextureCount += 2;
					}

					// when this is even the blur is longer vertically
					m_BloomTextureCount = m_BloomTextureCount - 1;

					m_BloomTextures = new unsigned int[m_BloomTextureCount];
					for (int texInd = 0; texInd < m_BloomTextureCount; ++texInd) {
						m_BloomTextures[texInd] = 0;
					}
				}

				glGenTextures(m_BloomTextureCount, m_BloomTextures);
				{
					// creating downscaled pairs of textures
					size_t texWidth = scrWidth, texHeight = scrHeight;
					for (size_t stepIter = 0; stepIter < m_BloomTextureCount; stepIter += 2) {
						// horizontal and vertical textures
						for (size_t axisIter = 0; axisIter < 2 && stepIter + axisIter < m_BloomTextureCount; ++axisIter) {
							glBindTexture(GL_TEXTURE_2D, m_BloomTextures[stepIter+axisIter]);
							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, nullptr);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
							glBindTexture(GL_TEXTURE_2D, 0);
						}

						texWidth = texWidth / 2;
						texHeight = texHeight / 2;
					}
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
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_RGB, scrWidth, scrHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_RenderBuffers[0]);

			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffers[1]);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_RGBA32F, scrWidth, scrHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, m_RenderBuffers[1]);

			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffers[2]);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_samples, GL_DEPTH24_STENCIL8, scrWidth, scrHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffers[2]);

			glDrawBuffers(2, colorAttachments);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				utils::printError("Failed to create main frame buffer!");
			}
		}

		// bloom framebuffers
		{
			if (m_BloomFramebuffers != nullptr)
				utils::fatalError("Bloom framebuffers aren't deallocating properly");

			m_BloomFramebuffers = new unsigned int[m_BloomTextureCount];
			glGenFramebuffers(m_BloomTextureCount, m_BloomFramebuffers);

			unsigned int colorAttachments[] = { GL_COLOR_ATTACHMENT0 };
			
			for (int i = 0; i < m_BloomTextureCount; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFramebuffers[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BloomTextures[i], 0);

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
		m_PostprocessingShader.buildShaders("shaders/postprocessing.shader", true);
		m_BlurShader.buildShaders("shaders/blur.shader", true);
		m_MixShader.buildShaders("shaders/mix.shader", true);
		m_SkyboxShader.buildShaders("shaders/skybox.shader", true);
		m_CelestialShader.buildShaders("shaders/celestial.shader", true);
		m_StarShader.buildShaders("shaders/star.shader", true);
		m_LineShader.buildShaders("shaders/line.shader", true);
	}
}