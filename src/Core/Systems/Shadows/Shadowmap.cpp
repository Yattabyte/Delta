#include "Systems\Shadows\Shadowmap.h"
#include "Systems\Message_Manager.h"
#include "Utilities\Engine_Package.h"
#include <minmax.h>

class SM_ShadowSizeRegularChangeCallback : public Callback_Container {
public:
	~SM_ShadowSizeRegularChangeCallback() {};
	SM_ShadowSizeRegularChangeCallback(System_Shadowmap *pointer) : m_pointer(pointer) {}
	void Callback(const float &value) {
		m_pointer->SetSize(SHADOW_REGULAR, value);
	}
private:
	System_Shadowmap *m_pointer;
};
class SM_ShadowSizeLargeChangeCallback : public Callback_Container {
public:
	~SM_ShadowSizeLargeChangeCallback() {};
	SM_ShadowSizeLargeChangeCallback(System_Shadowmap *pointer) : m_pointer(pointer) {}
	void Callback(const float &value) {
		m_pointer->SetSize(SHADOW_LARGE, value);
	}
private:
	System_Shadowmap *m_pointer;
};
class SM_ShadowQualityChangeCallback : public Callback_Container {
public:
	~SM_ShadowQualityChangeCallback() {};
	SM_ShadowQualityChangeCallback(System_Shadowmap *pointer) : m_pointer(pointer) {}
	void Callback(const float &value) {
		m_pointer->SetUpdateQuality(value);
	}
private:
	System_Shadowmap *m_pointer;
};

System_Shadowmap::~System_Shadowmap()
{
	if (m_Initialized) {
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SHADOW_SIZE_REGULAR, m_RegularChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SHADOW_SIZE_LARGE, m_largeChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SHADOW_QUALITY, m_QualityChangeCallback);
		delete m_RegularChangeCallback;
		delete m_largeChangeCallback;
		delete m_QualityChangeCallback;

		glDeleteFramebuffers(SHADOW_MAX, m_shadow_fbo);
		glDeleteTextures(SHADOW_MAX, m_shadow_depth);
		m_freed_shadow_spots[0].clear();
		m_freed_shadow_spots[1].clear();
	}
}

System_Shadowmap::System_Shadowmap()
{
	for (int x = 0; x < SHADOW_MAX; ++x) {
		m_shadow_fbo[x] = 0;
		m_shadow_depth[x] = 0;
		m_shadow_worldpos[x] = 0;
		m_shadow_worldnormal[x] = 0;
		m_shadow_radiantflux[x] = 0;
		m_shadow_count[x] = 0;
	}	
}

void System_Shadowmap::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_RegularChangeCallback = new SM_ShadowSizeRegularChangeCallback(this);
		m_largeChangeCallback = new SM_ShadowSizeLargeChangeCallback(this);
		m_QualityChangeCallback = new SM_ShadowQualityChangeCallback(this);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SHADOW_SIZE_REGULAR, m_RegularChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SHADOW_SIZE_LARGE, m_largeChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SHADOW_QUALITY, m_QualityChangeCallback);
		float size_regular = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SHADOW_SIZE_REGULAR);
		float size_large = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SHADOW_SIZE_LARGE);
		m_update_quality = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SHADOW_QUALITY);
		m_size[SHADOW_REGULAR] = vec2(max(1.0f, size_regular));
		m_size[SHADOW_LARGE] = vec2(max(1.0f, size_large));

		for (int x = 0; x < SHADOW_MAX; ++x) {
			glGenFramebuffers(1, &m_shadow_fbo[x]);
			glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_fbo[x]);

			// Create the depth buffer
			glGenTextures(1, &m_shadow_depth[x]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_depth[x]);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[x].x, m_size[x].y, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadow_depth[x], 0);

			// Create the World Position buffer
			glGenTextures(1, &m_shadow_worldpos[x]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldpos[x]);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_shadow_worldpos[x], 0);

			// Create the World Normal buffer
			glGenTextures(1, &m_shadow_worldnormal[x]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldnormal[x]);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_shadow_worldnormal[x], 0);

			// Create the Radiant Flux buffer
			glGenTextures(1, &m_shadow_radiantflux[x]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_radiantflux[x]);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[x].x, m_size[x].y, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_shadow_radiantflux[x], 0);

			GLenum Buffers[] = { GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, Buffers);

			GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
				std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
				MSG::Error(FBO_INCOMPLETE, "Shadowmap Manager", errorString);
				return;
			}
		}
		m_Initialized = true;
	}
}

void System_Shadowmap::Update(const float & deltaTime)
{
}

void System_Shadowmap::Update_Threaded(const float & deltaTime)
{
}

void System_Shadowmap::RegisterShadowCaster(const int & shadow_type, int & array_spot)
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
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_depth[shadow_type]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldpos[shadow_type]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldnormal[shadow_type]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_radiantflux[shadow_type]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[shadow_type].x, m_size[shadow_type].y, m_shadow_count[shadow_type], 0, GL_RGB, GL_FLOAT, NULL);

}

void System_Shadowmap::UnRegisterShadowCaster(const int & shadow_type, int & array_spot)
{
	bool found = false;
	for (int x = 0, size = m_freed_shadow_spots[shadow_type].size(); x < size; ++x)
		if (m_freed_shadow_spots[shadow_type][x] == array_spot)
			found = true;
	if (!found)
		m_freed_shadow_spots[shadow_type].push_back(array_spot);
}

void System_Shadowmap::BindForWriting(const int & ShadowSpot)
{
	glViewport(0, 0, m_size[ShadowSpot].x, m_size[ShadowSpot].y);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadow_fbo[ShadowSpot]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void System_Shadowmap::BindForReading(const int & ShadowSpot, const GLuint & ShaderTextureUnit)
{
	glActiveTexture(GL_TEXTURE0 + ShaderTextureUnit);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_depth[ShadowSpot]);
}

void System_Shadowmap::SetSize(const unsigned int & spot, const float & size)
{
	m_size[spot] = vec2(max(size, 1));
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_fbo[spot]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_depth[spot]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size[spot].x, m_size[spot].y, m_shadow_count[spot], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadow_depth[spot], 0);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldpos[spot]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[spot].x, m_size[spot].y, m_shadow_count[spot], 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_shadow_worldpos[spot], 0);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_worldnormal[spot]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[spot].x, m_size[spot].y, m_shadow_count[spot], 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_shadow_worldnormal[spot], 0);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_radiantflux[spot]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, m_size[spot].x, m_size[spot].y, m_shadow_count[spot], 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_shadow_radiantflux[spot], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void System_Shadowmap::SetUpdateQuality(const float & quality)
{
	m_update_quality = max(-1, quality);
}
