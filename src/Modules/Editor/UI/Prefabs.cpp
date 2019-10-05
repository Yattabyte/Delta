#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/component_types.h"
#include "Modules/World/World_M.h"
#include "imgui.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>


Prefabs::Prefabs(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_open = true;

	// Load Assets
	m_texBack = Shared_Texture(engine, "Editor//folderBack.png");
	m_texFolder = Shared_Texture(engine, "Editor//folder.png");
	m_texMissingThumb = Shared_Texture(engine, "Editor//prefab.png");
	m_texIconRefresh = Shared_Texture(engine, "Editor//iconRefresh.png");

	// Load prefabs
	populatePrefabs();
}

void Prefabs::tick(const float& deltaTime)
{
	if (m_open) {
		ImGui::SetNextWindowDockID(ImGui::GetID("LeftDock"), ImGuiCond_FirstUseEver);
		enum PrefabOptions {
			none,
			open,
			del,
			rename
		} prefabOption = none;
		// Draw Prefabs window
		if (ImGui::Begin("Prefabs", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
			static ImGuiTextFilter filter;
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
			filter.Draw("Search");
			auto alignOffset = ImGui::GetWindowContentRegionMax().x - 19.0f;
			alignOffset = alignOffset < 0.0f ? 0.0f : alignOffset;
			ImGui::SameLine(alignOffset);
			if (ImGui::ImageButton((ImTextureID)static_cast<uintptr_t>(m_texIconRefresh->existsYet() ? m_texIconRefresh->m_glTexID : 0), { 15, 15 }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
				populatePrefabs(m_prefabSubDirectory);
			ImGui::PopStyleVar();
			ImGui::Separator();
			ImGui::Spacing();
			const auto directory = "\\Prefabs\\" + m_prefabSubDirectory;
			ImGui::Text(directory.c_str());
			ImGui::Spacing();
			ImGuiStyle& style = ImGui::GetStyle();

			auto columnCount = int(float(ImGui::GetWindowContentRegionMax().x) / float((ImGui::GetStyle().ItemSpacing.x * 2) + 50));
			columnCount < 1 ? 1 : columnCount;
			ImGui::Columns(columnCount, nullptr, false);
			int count(0);
			for each (const auto & prefab in m_prefabs) {
				if (filter.PassFilter(prefab.name.c_str())) {
					ImGui::PushID(&prefab);
					GLuint textureID = 0;
					if (prefab.type == Entry::back && m_texBack->existsYet())
						textureID = m_texBack->m_glTexID;
					else if (prefab.type == Entry::folder && m_texFolder->existsYet())
						textureID = m_texFolder->m_glTexID;
					else if ((prefab.type == Entry::file) && m_texMissingThumb->existsYet())
						textureID = m_texMissingThumb->m_glTexID;

					ImGui::BeginGroup();
					ImVec4 color = count == m_selectedIndex ? ImVec4(1, 1, 1, 0.75) : ImVec4(0, 0, 0, 0);
					ImGui::ImageButton(
						(ImTextureID)static_cast<uintptr_t>(textureID),
						{ 50, 50 },
						{ 0.0f, 1.0f }, { 1.0f, 0.0f },
						-1,
						color
					);
					ImGui::TextWrapped(prefab.name.c_str());
					ImGui::EndGroup();
					if (ImGui::IsItemClicked()) {
						m_selectedIndex = count;
						if (ImGui::IsMouseDoubleClicked(0))
							prefabOption = open;
					}
					else if (ImGui::BeginPopupContextItem("Edit Prefab")) {
						m_selectedIndex = count;
						if (ImGui::MenuItem("Open")) { prefabOption = open; }
						ImGui::Separator();
						if (ImGui::MenuItem("Delete")) { prefabOption = del; }
						ImGui::Separator();
						if (ImGui::MenuItem("Rename")) { prefabOption = rename; }
						ImGui::EndPopup();
					}
					ImGui::PopID();
					ImGui::NextColumn();
				}
				count++;
			}
		}
		ImGui::End();

		// Do something with the option chosen
		bool openDelete = true, openRename = true;
		switch (prefabOption) {
		case open:
			openPrefabEntry();
			break;
		case del:
			ImGui::OpenPopup("Delete Prefab");
			break;
		case rename:
			ImGui::OpenPopup("Rename Prefab");
			break;
		}

		// Draw 'Delete Prefab' confirmation
		ImGui::SetNextWindowSize({ 350, 90 }, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Delete Prefab", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::TextWrapped(
				"Are you sure you want to delete this prefab?\n"
				"This won't remove the prefab from any levels."
			);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
			if (ImGui::Button("Delete", { 50, 20 })) {
				const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path);
				std::filesystem::remove(fullPath);
				m_selectedIndex = -1;
				populatePrefabs(m_prefabSubDirectory);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();
			ImGui::PopStyleColor(3);
			if (ImGui::Button("Cancel", { 75, 20 }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		// Draw 'Rename Prefab' dialogue
		if (ImGui::BeginPopupModal("Rename Prefab", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("Enter a new name for this prefab...");
			ImGui::Spacing();

			char nameInput[256];
			for (size_t x = 0; x < m_prefabs[m_selectedIndex].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
				nameInput[x] = m_prefabs[m_selectedIndex].name[x];
			nameInput[std::min(256ull, m_prefabs[m_selectedIndex].name.length())] = '\0';
			if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere(0);
			if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
				m_prefabs[m_selectedIndex].name = nameInput;
				const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path);
				std::filesystem::rename(fullPath, std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)));
				m_prefabs[m_selectedIndex].path = std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)).string();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}

void Prefabs::addPrefab(const std::vector<char>& entityData)
{
	size_t dataRead(0ull);
	const auto& [entityHandle, entity] = m_worldPreview.deserializeEntity(&entityData[0], entityData.size(), dataRead);
	m_prefabs.push_back({ "New Entity", m_prefabSubDirectory + "\\New Entity", Entry::file, entityHandle });
	m_selectedIndex = (int)(m_prefabs.size()) - 1;

	// Save Prefab to disk
	std::ofstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path, std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(entityData.data(), (std::streamsize)entityData.size());
	mapFile.close();
}

void Prefabs::populatePrefabs(const std::string& directory)
{
	// Reset prefab data
	m_prefabSubDirectory = directory;
	m_prefabs.clear();
	m_worldPreview.clear();

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\";
	const auto path = std::filesystem::path(rootPath + directory);

	// Add an entry to go back a folder
	if (directory != "" && directory != "." && directory != "..")
		m_prefabs.push_back(Entry{ "back", std::filesystem::relative(path.parent_path(), rootPath).string(), Entry::back, {} });

	// If in the root folder, add hard-coded prefab entries
	if (directory == "" || directory == ".") {
		//Basic Model Prefab
		{
			Transform_Component a;
			BoundingBox_Component b;
			Prop_Component c;
			a.m_localTransform.m_scale = glm::vec3(15.0f);
			a.m_localTransform.update();
			c.m_modelName = "FireHydrant\\FireHydrantMesh.obj";
			ecsBaseComponent* entityComponents[] = { &a, &b, &c };
			m_prefabs.push_back({ "Basic Model", "", Entry::file, m_worldPreview.makeEntity(entityComponents, 3ull, "Basic Model") });
		}
	}

	// Cycle through each entry on disk
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		Entry prefabEntry{ entry.path().filename().string(), std::filesystem::relative(entry, rootPath).string() };
		if (entry.is_regular_file()) {
			prefabEntry.type = Entry::file;
			std::ifstream prefabFile(entry, std::ios::binary | std::ios::beg);
			if (prefabFile.is_open()) {
				// To Do: Remove serial data from entry class
				const auto size = std::filesystem::file_size(entry);
				std::vector<char> data(size);
				prefabFile.read(&data[0], (std::streamsize)size);
				size_t dataRead(0ull);
				const auto& [entityHandle, entity] = m_worldPreview.deserializeEntity(&data[0], size, dataRead);
				prefabEntry.entityHandle = entityHandle;
			}
			prefabFile.close();
		}
		else if (entry.is_directory())
			prefabEntry.type = Entry::folder;
		m_prefabs.push_back(prefabEntry);
	}
}

void Prefabs::openPrefabEntry()
{
	const auto& selectedPrefab = m_prefabs[m_selectedIndex];
	if (selectedPrefab.type == Entry::back || selectedPrefab.type == Entry::folder) {
		const std::string nameCopy(selectedPrefab.path);
		populatePrefabs(nameCopy);
		m_selectedIndex = -1;
	}
	else
		m_editor->addEntity(m_worldPreview.serializeEntity(selectedPrefab.entityHandle));
}