#include "Modules/Editor/Systems/ClearSelection_System.h"


ClearSelection_System::ClearSelection_System(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	// Declare component types used
	addComponentType(Selected_Component::Runtime_ID);
}

void ClearSelection_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	auto& ecsWorld = m_editor.getWorld();
	for (const auto& componentParam : components)
		ecsWorld.removeEntityComponent((static_cast<Selected_Component*>(componentParam[0]))->m_entity, Selected_Component::Runtime_ID);
}