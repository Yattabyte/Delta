#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Engine.h"
#include "Systems\Graphics\Resources\VisualFX.h"
#include <random>


static void AssignTextureProperties(const GLuint & texID)
{
	glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Geometry_FBO::~Geometry_FBO()
{	
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(1, &m_noiseID);
		glDeleteTextures(2, m_texturesGB);
		glDeleteTextures(1, &m_depth_stencil);
		glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	}
}

Geometry_FBO::Geometry_FBO()
{		
	m_depth_stencil = 0;
	m_quadVAO = 0;
	m_quadVAOLoaded = false;
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;
	for (int x = 0; x < 2; ++x)
		m_texturesGB[x] = 0;
}

void Geometry_FBO::initialize(Engine * engine, VisualFX * visualFX)
{
	if (!m_Initialized) {
		// Default Parameters
		m_engine = engine;
		m_visualFX = visualFX;

		// Asset Loading
		m_engine->createAsset(m_shaderSSAO, std::string("FX\\SSAO"), true);
		m_engine->createAsset(m_shapeQuad, std::string("quad"), true);

		// Primitive Construction
		m_quadVAOLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint quadData[4] = { m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::ivec2(m_renderSize.x, f)); });

		// GL Loading
		initialize_noise();
		FrameBuffer::initialize();
		// Create the gbuffer textures
		glCreateTextures(GL_TEXTURE_2D, GBUFFER_NUM_TEXTURES, m_textures);
		for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
			glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			AssignTextureProperties(m_textures[x]);
			glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0 + x, m_textures[x], 0);
		}
		// Depth-stencil buffer texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_stencil);
		glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		AssignTextureProperties(m_depth_stencil);
		glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
		const GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		clear();
	}
}

void Geometry_FBO::initialize_noise()
{
	if (!m_Initialized) {
		glCreateTextures(GL_TEXTURE_2D, 2, m_texturesGB);
		for (int x = 0; x < 2; ++x) {
			glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
			AssignTextureProperties(m_texturesGB[x]);
		}

		// Prepare the noise texture and kernal	
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		glm::vec3 noiseArray[16];
		for (GLuint i = 0; i < 16; i++) {
			glm::vec3 noise( randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			noiseArray[i] = (noise);
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &m_noiseID);
		glTextureStorage2D(m_noiseID, 1, GL_RGB16F, 4, 4);
		glTextureSubImage2D(m_noiseID, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &noiseArray[0]);
		AssignTextureProperties(m_noiseID);
	}
}

void Geometry_FBO::clear()
{
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat clearDepth = 1.0f;
	GLint clearStencil = 0;
	glNamedFramebufferDrawBuffers(m_fbo, GBUFFER_NUM_TEXTURES, DrawBuffers);
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		glClearNamedFramebufferfv(m_fbo, GL_COLOR, x, clearColor);
	glClearNamedFramebufferfi(m_fbo, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
}

void Geometry_FBO::bindForWriting()
{
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glNamedFramebufferDrawBuffers(m_fbo, GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Geometry_FBO::bindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) 
		glBindTextureUnit(i, m_textures[i]);
	glBindTextureUnit(3, m_depth_stencil);
}

void Geometry_FBO::resize(const glm::ivec2 & size)
{
	FrameBuffer::resize(size);

	// Main textures
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0 + x, m_textures[x], 0);
	}

	// Depth-stencil buffer texture
	glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
	
	// Gaussian blur textures
	for (int x = 0; x < 2; ++x) 
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
}

void Geometry_FBO::end()
{
	// Return the borrowed depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
}

void Geometry_FBO::applyAO()
{
	if (m_shaderSSAO->existsYet() && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		//glBlendEquation(GL_FUNC_ADD);
		//glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

		glBindTextureUnit(1, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL]);
		glBindTextureUnit(2, m_noiseID);
		glBindTextureUnit(3, m_depth_stencil);

		m_shaderSSAO->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		glBlendFunc(GL_ONE, GL_ZERO);
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDisable(GL_BLEND);
		m_visualFX->applyGaussianBlur_Alpha(m_textures[GBUFFER_TEXTURE_TYPE_SPECULAR], m_texturesGB, m_renderSize, 5);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
}

void Geometry_FBO::bindDepthWriting()
{
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
}

void Geometry_FBO::bindDepthReading(const unsigned int & textureUnit)
{
	glBindTextureUnit(textureUnit, m_depth_stencil);
}
