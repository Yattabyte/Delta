#pragma once
#ifndef INDIRECTVISIBILITY_SYSTEM_H
#define INDIRECTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"


/***/
class IndirectVisibility_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~IndirectVisibility_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline explicit IndirectVisibility_System(const std::shared_ptr<Indirect_Light_Data>& frameData)
		: m_frameData(frameData) {
		addComponentType(Light_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Compile results PER viewport
		for (auto& viewInfo : m_frameData->viewInfo) {
			// Clear previous cached data
			viewInfo.lightIndices.clear();

			int index = 0;
			for each (const auto & componentParam in components)
				viewInfo.lightIndices.push_back((GLuint)index++);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<Indirect_Light_Data> m_frameData;
};

#endif // INDIRECTVISIBILITY_SYSTEM_H