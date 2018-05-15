#pragma once
#ifndef DIRECTIONAL_BASIC_TECH
#define DIRECTIONAL_BASIC_TECH
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\DS_Technique.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"

/**
* An interface for specific deferred shading lighting techniques.
* To be used only by the DS_Lighting class.
**/
class DT_ENGINE_API Directional_Tech_Cheap : public DS_Technique {
public:
	// (de)Constructors
	/** Destructor. */
	~Directional_Tech_Cheap();
	/** Constructor. */
	Directional_Tech_Cheap(Light_Buffers * lightBuffers);


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos);
	virtual void renderOcclusionCulling();
	virtual void renderShadows();
	virtual void renderLighting();


private:
	// Private Attributes
	VectorBuffer<Directional_Cheap_Struct> * m_lightSSBO; 
	Shared_Asset_Shader m_shader_Lighting;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_indirectShape;
	size_t m_size;
};

#endif // DIRECTIONAL_BASIC_TECH