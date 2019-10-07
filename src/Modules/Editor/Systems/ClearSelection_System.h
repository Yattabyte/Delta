#pragma once
#ifndef CLEARSELECTION_SYSTEM_H
#define CLEARSELECTION_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"


/** An ECS system responsible for deleting all Selected Components from entities. */
class ClearSelection_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~ClearSelection_System() = default;
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline ClearSelection_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		auto& ecsWorld = m_editor->getActiveWorld();
		for each (const auto & componentParam in components)
			ecsWorld.removeEntityComponent(((Selected_Component*)(componentParam[0]))->m_entity, Selected_Component::m_ID);
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // CLEARSELECTION_SYSTEM_H