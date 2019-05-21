#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Modules/Graphics/Common/FBO_Geometry.h"
#include "Modules/Graphics/Common/FBO_Lighting.h"
#include "Modules/Graphics/Common/FBO_Reflection.h"
#include "Modules/Graphics/Common/FBO_LightBounce.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Common/VisualFX.h"
#include "Modules/Graphics/ECS/Camera_C.h"
#include "Assets/Shader.h"
#include "Utilities/ECS/ecsSystem.h"
#include "Utilities/GL/VectorBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/MappedChar.h"


class ECS;

/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module : public Engine_Module {
public:
	// (de)Constructors
	~Graphics_Module();
	Graphics_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Render a single frame.
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods	
	/** Update the data for the specified camera. 
	@param	camera			the camera to update */
	void updateCamera(Camera_Buffer * camera);
	/** Set the specified camera index as the active camera. 
	@param	newCameraID		the camera index to use */
	void setActiveCamera(const GLuint & newCameraID);
	/** Return the index of the camera last active. 
	@return					the camera index of the last active camera */
	const GLuint getActiveCamera() const;
	/** Returns the active camera's data buffer.
	@return					the active camera's data buffer. */
	VB_Element<Camera_Buffer> * getActiveCameraBuffer();
	/** Returns the lighting buffer's FBO ID. 
	@return					the lighting buffer FBO ID. */
	const GLuint getLightingFBOID() const { return m_lightingFBO.m_fboID; };
	/** Returns the lighting buffer's texture ID.
	@return					the lighting buffer texture ID. */
	const GLuint getLightingTexID() const { return m_lightingFBO.m_textureID; };

	
private:
	// Private Attributes
	ECS *							m_ecs = nullptr;
	GLuint							m_activeCamera = 0;
	glm::ivec2						m_renderSize = glm::ivec2(1);
	ECSSystemList					m_renderingSystems;
	std::vector<GFX_Core_Effect*>	m_fxTechs;
	MappedChar<GFX_Core_Effect*>	m_mappedFX;
	std::shared_ptr<RH_Volume>		m_volumeRH;
	VisualFX						m_visualFX;
	Shared_Shader					m_shaderCull = Shared_Shader(), m_shaderGeometry = Shared_Shader();
	std::shared_ptr<bool>			m_aliveIndicator = std::make_shared<bool>(true);
	FBO_Geometry					m_geometryFBO;
	FBO_Lighting					m_lightingFBO;
	FBO_Reflection					m_reflectionFBO;
	FBO_LightBounce					m_bounceFBO;
	StaticBuffer					m_cameraIndexBuffer;
	VectorBuffer<Camera_Buffer>		m_cameraBuffer;
	VB_Element<Camera_Buffer> *		m_defaultCamera;
};

#endif // GRAPHICS_MODULE_H