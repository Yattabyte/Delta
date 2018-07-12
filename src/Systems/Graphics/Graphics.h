#pragma once
#ifndef SYSTEM_GRAPHICS_H
#define SYSTEM_GRAPHICS_H

#include "Systems\System_Interface.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Geometry_Technique.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\Lights\Light_Tech.h"
#include "Systems\Graphics\Resources\FX Techniques\FX_Technique.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Systems\Graphics\Resources\Geometry_Buffers.h"
#include "Systems\Graphics\Resources\VisualFX.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\MappedChar.h"
#include <vector>


class Engine;
class Camera;

/**
 * An engine system responsible for rendering. Creates Geometry_FBO, Lighting_FBO, and VisualFX
 * @note	performs physically based rendering techniques.
 **/
class System_Graphics : public System
{
public: 
	// (de)Constructors
	/** Destroy the rendering system. */
	~System_Graphics();
	/** Construct the rendering system. */
	System_Graphics();

	
	// Public Functions
	/** Returns a type-casted technique that matches the given name.
	 * @param	c	a const char array name of the desired technique to find
	 * @return		the technique requested */
	template <typename T> T * getBaseTech(const char * c) {
		return (T*)m_techMap[c];
	}
	/** Returns a type-casted lighting technique that matches the given name.
	 * @param	c	a const char array name of the desired technique to find
	 * @return		the technique requested */
	template <typename T> T * getLightingTech(const char * c) {
		return (T*)m_lightingTechMap[c];
	}
	

	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


	// Public Attributes
	// Frame Buffers
	Geometry_FBO	m_geometryFBO;
	Lighting_FBO	m_lightingFBO;
	Reflection_FBO	m_reflectionFBO;
	// Storage Buffers
	Light_Buffers	m_lightBuffers;
	Geometry_Buffers m_geometryBuffers;
	VectorBuffer<Reflection_Struct>	m_reflectionSSBO;
	

private:
	// Private Methods
	/** Regenerate the noise kernel. */
	void generateKernal();
	/** Sends data to GPU in one pass.
	 * For example, sending updated mat4's into buffers. */
	void send2GPU(const Visibility_Token & vis_token);
	/** Perform pre-passes and update data present on the GPU. 
	 * For example, performing GPU accelerated occlusion culling or shadow mapping. */
	void updateOnGPU(const Visibility_Token & vis_token);
	/** Render a single frame. */
	void renderFrame(const Visibility_Token & vis_token);


	// Private Attributes
	ivec2			m_renderSize;
	VisualFX		m_visualFX;
	StaticBuffer	m_userBuffer;
	bool			m_ssao;

	// Rendering Techniques
	vector<Geometry_Technique*> m_geometryTechs;
	vector<Lighting_Technique*> m_lightingTechs;
	vector<FX_Technique*>		m_fxTechs;
	// Base light type techniques
	vector<Light_Tech*>			m_baseTechs;
	MappedChar<void*>			m_techMap;
	MappedChar<void*>			m_lightingTechMap;
};

#endif // SYSTEM_GRAPHICS_H