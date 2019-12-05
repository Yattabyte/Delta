#pragma once
#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"
#include <memory>


/** An ECS system responsible for applying a transformation hierarchy between parent and child entities. */
class Transform_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Transform_System() = default;
	/** Construct this system.
	@param	engine		reference to the engine to use. */
	explicit Transform_System(Engine& engine) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


	// Public Attributes
	Engine& m_engine;
	ecsWorld* m_world = nullptr;
};

#endif // TRANSFORM_SYSTEM_H