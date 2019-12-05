#include "Modules/Graphics/Lighting/Indirect/IndirectVisibility_System.h"


IndirectVisibility_System::IndirectVisibility_System(Indirect_Light_Data& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void IndirectVisibility_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	// Compile results PER viewport
	for (auto& viewInfo : m_frameData.viewInfo) {
		// Clear previous cached data
		viewInfo.lightIndices.clear();

		int index = 0;
		for (const auto& componentParam : components)
			viewInfo.lightIndices.push_back((GLuint)index++);
	}
}