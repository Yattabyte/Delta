#pragma once
#ifndef SPOT_CHEAP_TECH_H
#define SPOT_CHEAP_TECH_H

#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Light_Tech.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\StaticBuffer.h"


class EnginePackage;

/**
 * A deferred shading lighting technique that manages cheap spot lights.
 **/
class Spot_Cheap_Tech : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Spot_Cheap_Tech();
	/** Constructor. */
	Spot_Cheap_Tech(Light_Buffers * lightBuffers);


	// Interface Implementations
	virtual const char * getName() const { return "Spot_Cheap_Tech"; }
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos);
	virtual void updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution) {}
	virtual void renderOcclusionCulling() {}
	virtual void renderShadows() {}
	virtual void renderLightBounce() {}
	virtual void renderLighting();


private:
	// Private Attributes
	VectorBuffer<Spot_Cheap_Struct> * m_lightSSBO;
	Shared_Asset_Shader m_shader_Lighting;
	Shared_Asset_Primitive m_shapeCone;
	GLuint m_coneVAO;
	bool m_coneVAOLoaded;
	DynamicBuffer m_visShapes;
	StaticBuffer m_indirectShape;
	size_t m_size;
};

#endif // SPOT_CHEAP_TECH_H