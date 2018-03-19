#include "Systems\Graphics\Frame Buffers\Shadow_Buffer.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include <minmax.h>


Shadow_Buffer::~Shadow_Buffer()
{
	if (m_Initialized) {
		m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_SIZE_REGULAR, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_SIZE_LARGE, this);

		m_freed_shadow_spots[0].clear();
		m_freed_shadow_spots[1].clear();
		glDeleteTextures(1, m_shadow_radiantflux);
		glDeleteTextures(1, m_shadow_worldnormal);
		glDeleteTextures(1, m_shadow_worldpos);
		glDeleteTextures(1, m_shadow_depth);
		glDeleteFramebuffers(SHADOW_MAX, m_shadow_fbo);
	}
}

Shadow_Buffer::Shadow_Buffer()
{
	m_Initialized = false;
	for (int x = 0; x < SHADOW_MAX; ++x) {
		m_shadow_fbo[x] = 0;
		m_shadow_depth[x] = 0;
		m_shadow_worldpos[x] = 0;
		m_shadow_worldnormal[x] = 0;
		m_shadow_radiantflux[x] = 0;
		m_shadow_count[x] = 0;
	}	
}

void Shadow_Buffer::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_size[SHADOW_REGULAR].x = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_SIZE_REGULAR, this, [&](const float &f) {setSize(SHADOW_REGULAR, f); });
		m_size[SHADOW_LARGE].x = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_SIZE_LARGE, this, [&](const float &f) {setSize(SHADOW_LARGE, f); });
		m_size[SHADOW_REGULAR] = vec2(max(1.0f, m_size[SHADOW_REGULAR].x));
		m_size[SHADOW_LARGE] = vec2(max(1.0f, m_size[SHADOW_LARGE].x));

		glGenFramebuffers(SHADOW_MAX, m_shadow_fbo);
		glGenTextures(SHADOW_MAX, m_shadow_depth);
		glGenTextures(SHADOW_MAX, m_shadow_worldpos);
		glGenTextures(SHADOW_MAX, m_shadow_worldnormal);
		glGenTextures(SHADOW_MAX, m_shadow_radiantflux);
		for (int x = 0; x < SHADOW_MAX; ++x) {
			// Create the depth buffer
			glTextureImage3DEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[x].x, m_size[x].y, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteriEXT(m_shadow_depth[x], GL_TEXTURE_2D_ARRAY, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
			glNamedFramebufferTextureEXT(m_shadow_fbo[x], GL_DEPTH_ATTACHMENT, m_shadow_depth[x], 0);

			// Create the World Position buffer
			glTextureImage3DEXT(m_shadow_worldpos[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureParameteriEXT(m_shadow_worldpos[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_worldpos[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_worldpos[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteriEXT(m_shadow_worldpos[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glNamedFramebufferTextureEXT(m_shadow_fbo[x], GL_COLOR_ATTACHMENT0, m_shadow_worldpos[x], 0);

			// Create the World Normal buffer
			glTextureImage3DEXT(m_shadow_worldnormal[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureParameteriEXT(m_shadow_worldnormal[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_worldnormal[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_worldnormal[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteriEXT(m_shadow_worldnormal[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glNamedFramebufferTextureEXT(m_shadow_fbo[x], GL_COLOR_ATTACHMENT1, m_shadow_worldnormal[x], 0);

			// Create the Radiant Flux buffer
			glTextureImage3DEXT(m_shadow_radiantflux[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureParameteriEXT(m_shadow_radiantflux[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_radiantflux[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteriEXT(m_shadow_radiantflux[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteriEXT(m_shadow_radiantflux[x], GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glNamedFramebufferTextureEXT(m_shadow_fbo[x], GL_COLOR_ATTACHMENT2, m_shadow_radiantflux[x], 0);

			GLenum Buffers[] = {GL_COLOR_ATTACHMENT0,
								GL_COLOR_ATTACHMENT1,
								GL_COLOR_ATTACHMENT2};
			glNamedFramebufferDrawBuffers(m_shadow_fbo[x], 3, Buffers);

			GLenum Status = glCheckNamedFramebufferStatus(m_shadow_fbo[x], GL_FRAMEBUFFER);
			if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
				std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
				MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Shadowmap Manager", errorString);
				return;
			}
		}
		m_Initialized = true;
	}
}

void Shadow_Buffer::registerShadowCaster(const int & shadow_type, int & array_spot)
{
	if (m_freed_shadow_spots[shadow_type].size()) {
		array_spot = m_freed_shadow_spots[shadow_type].front();
		m_freed_shadow_spots[shadow_type].pop_front();
	}
	else {
		array_spot = m_shadow_count[shadow_type];
		m_shadow_count[shadow_type]++;
	}

	// Adjust the layer count every time a new light is added (preserve memory rather than preallocating memory for shadows that don't exist	
	glTextureImage3DEXT(m_shadow_depth[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTextureImage3DEXT(m_shadow_worldpos[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);	
	glTextureImage3DEXT(m_shadow_worldnormal[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glTextureImage3DEXT(m_shadow_radiantflux[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
}

void Shadow_Buffer::unregisterShadowCaster(const int & shadow_type, int & array_spot)
{
	bool found = false;
	for (int x = 0, size = m_freed_shadow_spots[shadow_type].size(); x < size; ++x)
		if (m_freed_shadow_spots[shadow_type][x] == array_spot)
			found = true;
	if (!found)
		m_freed_shadow_spots[shadow_type].push_back(array_spot);
}

void Shadow_Buffer::bindForWriting(const int & shadow_type)
{
	glViewport(0, 0, m_size[shadow_type].x, m_size[shadow_type].y);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadow_fbo[shadow_type]);
}

void Shadow_Buffer::bindForReading(const int & shadow_type, const GLuint & ShaderTextureUnit)
{
	glBindMultiTextureEXT(GL_TEXTURE0 + ShaderTextureUnit, GL_TEXTURE_2D_ARRAY, m_shadow_depth[shadow_type]);
}

void Shadow_Buffer::BindForReading_GI(const int & ShadowSpot, const GLuint & ShaderTextureUnit)
{
	glBindMultiTextureEXT(ShaderTextureUnit, GL_TEXTURE_2D_ARRAY, m_shadow_worldpos[ShadowSpot]);
	glBindMultiTextureEXT(ShaderTextureUnit + 1, GL_TEXTURE_2D_ARRAY, m_shadow_worldnormal[ShadowSpot]);
	glBindMultiTextureEXT(ShaderTextureUnit + 2, GL_TEXTURE_2D_ARRAY, m_shadow_radiantflux[ShadowSpot]);
}

void Shadow_Buffer::clearShadow(const int & shadow_type, const int & layer)
{
	const float clearDepth(1.0f);
	const vec3 clear(0.0f);
	glClearTexSubImage(m_shadow_depth[shadow_type], 0, 0, 0, layer, m_size[shadow_type].x, m_size[shadow_type].y, 1.0f, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	glClearTexSubImage(m_shadow_worldpos[shadow_type], 0, 0, 0, layer, m_size[shadow_type].x, m_size[shadow_type].y, 1.0f, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_shadow_worldnormal[shadow_type], 0, 0, 0, layer, m_size[shadow_type].x, m_size[shadow_type].y, 1.0f, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_shadow_radiantflux[shadow_type], 0, 0, 0, layer, m_size[shadow_type].x, m_size[shadow_type].y, 1.0f, GL_RGB, GL_FLOAT, &clear);
}

void Shadow_Buffer::setSize(const unsigned int & shadow_type, const float & size)
{
	m_size[shadow_type] = vec2(max(size, 1));

	glTextureImage3DEXT(m_shadow_depth[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glNamedFramebufferTextureEXT(m_shadow_fbo[shadow_type], GL_DEPTH_ATTACHMENT, m_shadow_depth[shadow_type], 0);

	glTextureImage3DEXT(m_shadow_worldpos[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTextureEXT(m_shadow_fbo[shadow_type], GL_COLOR_ATTACHMENT0, m_shadow_worldpos[shadow_type], 0);

	glTextureImage3DEXT(m_shadow_worldnormal[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTextureEXT(m_shadow_fbo[shadow_type], GL_COLOR_ATTACHMENT1, m_shadow_worldnormal[shadow_type], 0);

	glTextureImage3DEXT(m_shadow_radiantflux[shadow_type], GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTextureEXT(m_shadow_fbo[shadow_type], GL_COLOR_ATTACHMENT2, m_shadow_radiantflux[shadow_type], 0);
}

vec2 Shadow_Buffer::getSize(const unsigned int & shadow_type)
{
	return m_size[shadow_type];
}
