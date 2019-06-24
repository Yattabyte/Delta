#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/CameraFollower_System.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/MappedChar.h"


/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this graphics rendering module. */
	~Graphics_Module();
	/** Construct a graphics rendering module. */
	inline Graphics_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Render a single frame.
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/***/
	void addPerViewportSystem(BaseECSSystem * system);
	/** Render using our graphics pipeline, from the camera buffer specified into the framebuffers and volume specified.
	@param	deltaTime		the amount of time since last frame.
	@param	viewport		the view port to render into.
	@param	categories		the technique categories to allow for rendering, defaults to ALL. */
	void render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const unsigned int & allowedCategories = Graphics_Technique::ALL);
	/** Returns a shared pointer to the primary viewport.
	@return					the primary viewport. */
	inline std::shared_ptr<Viewport> getPrimaryViewport() const {
		return m_viewport;
	}

	
private:
	// Private Attributes
	glm::ivec2								m_renderSize = glm::ivec2(1);
	FrustumCull_System						m_frustumCuller;
	CameraFollower_System					m_cameraFollower;
	ECSSystemList							m_systems;
	std::unique_ptr<Graphics_Pipeline>		m_pipeline;
	std::shared_ptr<Viewport>				m_viewport;
	std::shared_ptr<bool>					m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_MODULE_H