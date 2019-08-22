#pragma once
#ifndef INSPECTOR_TRANSFORM_SYSTEM_H
#define INSPECTOR_TRANSFORM_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_Transform_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_Transform_System(LevelEditor_Module* editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto & selectedEntities = m_editor->getSelection();
		std::vector<Transform_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (Transform_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static void* previousSelection = nullptr, * currentSelection = selectedComponents[0];
			static bool selectionChanged = false;
			if (previousSelection != currentSelection) {
				previousSelection = currentSelection;
				selectionChanged = true;
			}
			const auto text = Transform_Component::STRING_NAME + ": (" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				static bool treatAsGroup = true;
				ImGui::Checkbox("Treat as group", &treatAsGroup);
				if (treatAsGroup) {
					const auto groupCenter = find_center_transform(selectedComponents);
					auto posInput = groupCenter.m_position;
					if (ImGui::DragFloat3("Position", glm::value_ptr(posInput))) {
						const auto delta = posInput - groupCenter.m_position;
						for each (auto & component in selectedComponents) {
							component->m_localTransform.m_position += delta;
							component->m_localTransform.update();
						}
					}
				}
				else {
					auto posInput = selectedComponents[0]->m_localTransform.m_position;
					if (ImGui::DragFloat3("Position", glm::value_ptr(posInput)))
						for each (auto & component in selectedComponents) {
							component->m_localTransform.m_position = posInput;
							component->m_localTransform.update();
						}					
				}
				auto sclInput = selectedComponents[0]->m_localTransform.m_scale;
				if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput)))
					for each (auto & component in selectedComponents) {
						component->m_localTransform.m_scale = sclInput;
						component->m_localTransform.update();
					}				
				auto rotInput = glm::degrees(glm::eulerAngles(selectedComponents[0]->m_localTransform.m_orientation));
				if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotInput)))
					for each (auto & component in selectedComponents) {
						component->m_localTransform.m_orientation = glm::quat(glm::radians(rotInput));
						component->m_localTransform.update();
					}			
				m_editor->setGizmoTransform(selectedComponents[0]->m_localTransform);
			}
		}
	}


private:
	// Private Methods
	/***/
	static Transform find_center_transform(const std::vector<Transform_Component*> & transforms) {
		Transform center;
		for each (const auto & component in transforms)
			center *= component->m_localTransform;
		center.m_position /= transforms.size();
		center.update();
		return center;
	}


	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_TRANSFORM_SYSTEM_H