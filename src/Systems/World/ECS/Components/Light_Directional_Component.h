#pragma once
#ifndef LIGHT_DIRECTIONAL_COMPONENT
#define LIGHT_DIRECTIONAL_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define LIGHT_TYPE_DIRECTIONAL 0
#define NUM_CASCADES 4
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"

using namespace glm;
class Light_Directional_Creator;
class EnginePackage;
class Shadow_FBO;


/**
 * A renderable light component that mimics the sun.
 * Uses cascaded shadowmaps.
 **/
class DT_ENGINE_API Light_Directional_Component : protected Lighting_Component
{
public:
	// Interface implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void directPass(const int &vertex_count);
	virtual void indirectPass();
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position) const;


	// Public Methods
	/** Sends current data to the GPU. */
	void update();
	/** Recalculates the shadowmap cascades. */
	void calculateCascades();


protected:
	// (de)Constructors
	/** Destroys a directional light component. */
	~Light_Directional_Component();
	/** Constructs a directional light component. */
	Light_Directional_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
	mat4 m_mMatrix;
	float m_cascadeEnd[5];
	int m_shadowSpots[NUM_CASCADES];
	EnginePackage *m_enginePackage;
	Shadow_FBO *m_shadowMapper;
	Camera m_camera;
	DynamicBuffer m_visGeoUBO, m_indirectGeo;
	friend class Light_Directional_Creator;
};

class DT_ENGINE_API Light_Directional_Creator : public ComponentCreator
{
public:
	Light_Directional_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Directional_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_COMPONENT