#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/World_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Prefabs::Prefabs(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	// Load prefabs
	populatePrefabs();
}

void Prefabs::tick(const float & deltaTime)
{
	ImGui::SetNextWindowDockID(ImGui::GetID("LeftDock"), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Prefabs", NULL)) {
		static ImGuiTextFilter filter;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		filter.Draw("Search");
		ImGui::PopStyleVar();
		// Loop over all prefabs
		if (filter.PassFilter("Fire Hydrant")) {
			if (ImGui::Button("Fire Hydrant")) {
				spawnPrefab(0);
			}
		}
	}
	ImGui::End();
}

void Prefabs::populatePrefabs()
{
}

void Prefabs::spawnPrefab(const int & index)
{
	auto & world = m_engine->getModule_World();

	// Dynamic Prop Entity
	Renderable_Component renderable;
	BoundingSphere_Component bsphere;
	Prop_Component prop;
	prop.m_modelName = "FireHydrant\\FireHydrantMesh.obj";
	prop.m_skin = 0;

	// Later, we must perform some check for a transform component, adding it if its missing
	// Similarly, later I'd like to do a check for a prop component, if missing add a sprite billboard component (renders only in the editor)
	Transform_Component trans;
	trans.m_localTransform.m_position = m_editor->getGizmoPosition();
   	trans.m_localTransform.update();
	BaseECSComponent * entityComponents[] = { &renderable, &bsphere, &prop, &trans };
	const int types[] = { Renderable_Component::ID, BoundingSphere_Component::ID, Prop_Component::ID, Transform_Component::ID };
	world.makeEntity(entityComponents, types, 4ull, "Fire Hydrant Prefab");
}