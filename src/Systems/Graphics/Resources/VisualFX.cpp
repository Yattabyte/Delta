#include "Systems\Graphics\Resources\VisualFX.h"
#include "Utilities\EnginePackage.h"


VisualFX::~VisualFX()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

VisualFX::VisualFX()
{
	m_Initialized = false;
	m_vaoLoaded = false;
	m_quadVAO = 0;
	m_fbo_GB = 0;
}

void VisualFX::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		m_vaoLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_vaoLoaded = true; });
		GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);

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
	glCreateFramebuffers(1, &m_fbo_GB);

	Asset_Loader::load_asset(m_shaderGB, "FX\\gaussianBlur");
	Asset_Loader::load_asset(m_shaderGB_A, "FX\\gaussianBlur_Alpha");
}

void VisualFX::applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->existsYet() && m_vaoLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->bind();
		m_shaderGB->Set_Uniform(0, horizontal);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTextureUnit(0, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->Set_Uniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTextureUnit(0, flipTextures[!horizontal]);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB->Set_Uniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void VisualFX::applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->existsYet() && m_vaoLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		m_shaderGB_A->bind();
		m_shaderGB_A->Set_Uniform(0, horizontal);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTextureUnit(0, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->Set_Uniform(0, horizontal);

			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTextureUnit(0, flipTextures[!horizontal]);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB_A->Set_Uniform(0, horizontal);

		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}