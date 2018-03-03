#include "Systems\Graphics\VisualFX.h"
#include "Utilities\EnginePackage.h"


VisualFX::~VisualFX()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(Asset::FINALIZED, this);
}

VisualFX::VisualFX()
{
	m_Initialized = false;
	m_quadVAO = 0;
	m_fbo_GB = 0;
}

void VisualFX::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_shapeQuad->addCallback(Asset::FINALIZED, this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });

		initializeCubeFilter();
		initializeGausianBlur();

		m_Initialized = true;
	}
}

void VisualFX::initializeCubeFilter()
{
}

void VisualFX::initializeGausianBlur()
{
	glGenFramebuffers(1, &m_fbo_GB);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_GB);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Asset_Loader::load_asset(m_shaderGB, "FX\\gaussianBlur");
	Asset_Loader::load_asset(m_shaderGB_A, "FX\\gaussianBlur_Alpha");
}

void VisualFX::applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->bind();
		m_shaderGB->Set_Uniform(0, horizontal);

		glBindVertexArray(m_quadVAO);
		const int quad_size = m_shapeQuad->getSize();
		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->Set_Uniform(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB->Set_Uniform(0, horizontal);

		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		glBindVertexArray(0);
		Asset_Shader::Release();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void VisualFX::applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		m_shaderGB_A->bind();
		m_shaderGB_A->Set_Uniform(0, horizontal);

		glBindVertexArray(m_quadVAO);
		const int quad_size = m_shapeQuad->getSize();
		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->Set_Uniform(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB_A->Set_Uniform(0, horizontal);

		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		glBindVertexArray(0);
		Asset_Shader::Release();
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}