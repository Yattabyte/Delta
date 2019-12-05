#pragma once
#ifndef FRUSTUMCULL_SYSTEM_H
#define FRUSTUMCULL_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for frustum culling all render-able components with a bounding sphere and a position. */
class FrustumCull_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~FrustumCull_System() = default;
	/** Construct this system.
	@param	sceneCameras	reference to the scene cameras to use. */
	explicit FrustumCull_System(std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	std::vector<Camera*>& m_sceneCameras;
};

#endif // FRUSTUMCULL_SYSTEM_H