#include "Modules/Editor/UI/OpenDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Modules/UI/dear imgui/imgui_internal.h"
#include "Engine.h"
#include <chrono>
#include <filesystem>
#include <sstream>
#include <time.h>


OpenDialogue::OpenDialogue(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_open = false;
	m_iconFile = Shared_Texture(engine, "Editor//iconFile.png");
	m_iconFolder = Shared_Texture(engine, "Editor//iconFolder.png");
	m_iconBack = Shared_Texture(engine, "Editor//iconBack.png");
	m_iconRefresh = Shared_Texture(engine, "Editor//iconRefresh.png");
}

void OpenDialogue::tick(const float& deltaTime)
{
	tickMainDialogue();
}

void OpenDialogue::populateLevels(const std::string& directory)
{
	m_levels.clear();
	m_subDirectory = directory;

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\";
	const auto path = std::filesystem::path(rootPath + directory);
	if (directory != "" && directory != "." && directory != "..")
		m_levels.push_back(LevelEntry{ "..", std::filesystem::relative(path.parent_path(), rootPath).string(), "", "", "", "", LevelEntry::back });
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		std::string timeString = "";
		struct _stat64 fileInfo;
		if (_wstati64(entry.path().wstring().c_str(), &fileInfo) == 0) {
			const auto t = std::localtime(&fileInfo.st_mtime);
			timeString = std::to_string(t->tm_hour > 12 ? 24 - t->tm_hour : t->tm_hour) + ":" + std::to_string(t->tm_min)
				+ (t->tm_hour >= 12 ? "PM " : "AM ") + std::to_string(t->tm_mday) + "/" + std::to_string(t->tm_mon) + "/" + std::to_string(t->tm_year - 100);
		}
		constexpr static auto readableFileSize = [](const size_t& size) -> std::string {
			auto remainingSize = (double)size;
			constexpr static char* units[] = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };
			int i = 0;
			while (remainingSize > 1024.00) {
				remainingSize /= 1024.00;
				++i;
			}
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << remainingSize;
			return stream.str() + units[i];
		};
		LevelEntry prefabEntry{
			entry.path().filename().stem().string(),
			std::filesystem::relative(entry, rootPath).string(),
			entry.path().extension().string(),
			"File",
			timeString,
			readableFileSize(entry.file_size())
		};
		if (entry.is_regular_file()) {
			prefabEntry.type = LevelEntry::file;
			if (prefabEntry.extension == ".bmap")
				prefabEntry.extType = "Map";
			else if (prefabEntry.extension == ".autosave")
				prefabEntry.extType = "Autosave";
		}
		else if (entry.is_directory()) {
			prefabEntry.name = prefabEntry.name;
			prefabEntry.type = LevelEntry::folder;
			prefabEntry.extType = "Folder";
		}
		m_levels.push_back(prefabEntry);
	}
}

void OpenDialogue::tickMainDialogue()
{
	static bool freshlyOpened = true; // flag used for operations that should happen only once-per-opening
	const auto title = "Open Level";
	if (m_open)
		ImGui::OpenPopup(title);
	ImGui::SetNextWindowSize({ 600, 500 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		enum DialogueOptions {
			none,
			use,
			clone,
			rename,
			del,
		} option = none;
		if (freshlyOpened)
			populateLevels();

		// Header
		ImGui::Text("Choose a level to open...");
		ImGui::SameLine(std::max(ImGui::GetWindowContentRegionMax().x - 28.0f, 0.0f));
		if (ImGui::ImageButton((ImTextureID)static_cast<uintptr_t>(m_iconRefresh->existsYet() ? m_iconRefresh->m_glTexID : 0), { 15, 15 }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
			populateLevels(m_subDirectory);
		ImGui::Spacing();

		// Display a list of level entries for the directory chosen
		ImGui::BeginChild("Level List", ImVec2(580, ImGui::GetWindowContentRegionMax().y - 85), true);
		ImGui::Text("Name");
		ImGui::SameLine(250);
		ImGui::Text("Type");
		ImGui::SameLine(350);
		ImGui::Text("Date Modified");
		ImGui::SameLine(475);
		ImGui::Text("Size");
		ImGui::Separator();
		int index = 0;
		for each (const auto & level in m_levels) {
			GLuint icon = (level.type == LevelEntry::file && m_iconFile->existsYet()) ? m_iconFile->m_glTexID :
				(level.type == LevelEntry::folder && m_iconFolder->existsYet()) ? m_iconFolder->m_glTexID :
				(level.type == LevelEntry::back && m_iconBack->existsYet()) ? m_iconBack->m_glTexID : 0;
			ImGui::PushID(index);
			ImGui::Image((ImTextureID)static_cast<uintptr_t>(icon), ImVec2(15, 15), { 0.0f, 1.0f }, { 1.0f, 0.0f });
			ImGui::SameLine(0);
			const auto name = level.name + level.extension;
			ImGui::Selectable(name.c_str(), m_selected == index);
			if (ImGui::IsItemClicked()) {
				m_selected = index;
				if (ImGui::IsMouseDoubleClicked(0))
					option = use;
			}
			else if (ImGui::BeginPopupContextItem("Edit Level")) {
				m_selected = index;
				if (ImGui::MenuItem("Open")) { option = use; }
				ImGui::Separator();
				if (ImGui::MenuItem("Clone")) { option = clone; }
				if (ImGui::MenuItem("Rename")) { option = rename; }
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) { option = del; }
				ImGui::EndPopup();
			}
			ImGui::SameLine(250);
			ImGui::Text(level.extType.c_str());
			ImGui::SameLine(350);
			ImGui::Text(level.date.c_str());
			ImGui::SameLine(475);
			ImGui::Text(level.size.c_str());
			ImGui::PopID();
			index++;
		}
		ImGui::EndChild();
		ImGui::Spacing();

		// Display an open button
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, m_selected == -1);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * ((m_selected == -1) ? 0.25f : 1.0f));
		if (ImGui::Button("Open", { 75, 20 }))
			option = use;
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();

		// Display a cancel button
		if (ImGui::Button("Cancel", { 100, 20 })) {
			m_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		if (option == use) {
			const auto& selectedLevel = m_levels[m_selected];
			if (selectedLevel.type == LevelEntry::back || selectedLevel.type == LevelEntry::folder) {
				const std::string nameCopy(selectedLevel.path);
				populateLevels(nameCopy);
				m_selected = -1;
			}
			else {
				m_editor->openLevel(selectedLevel.path);
				m_selected = -1;
				m_open = false;
			}
		}

		if (option == clone) {
			const auto srcPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			auto dstPath = srcPath;
			int count = 1;
			while (std::filesystem::exists(dstPath))
				dstPath.replace_filename(
				(srcPath.has_stem() ? srcPath.stem().string() : srcPath.filename().string())
					+ " - Copy (" + std::to_string(count++) + ")"
					+ (srcPath.has_extension() ? srcPath.extension().string() : "")
				);
			std::filesystem::copy(srcPath, dstPath);
			populateLevels(m_subDirectory);
		}

		if (option == rename)
			ImGui::OpenPopup("Rename Level");
		tickRenameDialogue();

		if (option == del)
			ImGui::OpenPopup("Delete Level");
		tickDeleteDialogue();


		ImGui::EndPopup();
		freshlyOpened = false;
	}
	else
		freshlyOpened = true;
}

void OpenDialogue::tickRenameDialogue()
{
	bool openRename = true;
	if (ImGui::BeginPopupModal("Rename Level", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Enter a new name for this item...");
		ImGui::Spacing();
		char nameInput[256];
		for (size_t x = 0; x < m_levels[m_selected].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
			nameInput[x] = m_levels[m_selected].name[x];
		nameInput[std::min(256ull, m_levels[m_selected].name.length())] = '\0';
		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);
		if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_levels[m_selected].name = nameInput;
			if (m_levels[m_selected].type == LevelEntry::folder)
				m_levels[m_selected].name = "\\" + m_levels[m_selected].name + "\\";
			const auto oldPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			const auto newPath = std::filesystem::path(oldPath.parent_path().string() + "\\" + std::string(nameInput) + (oldPath.has_extension() ? oldPath.extension().string() : "")).string();
			std::filesystem::rename(oldPath, newPath);
			m_levels[m_selected].path = std::filesystem::relative(newPath, Engine::Get_Current_Dir() + "\\Maps\\").string();
			m_levels[m_selected].name = std::string(nameInput);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void OpenDialogue::tickDeleteDialogue()
{
	bool openDelete = true;
	ImGui::SetNextWindowSize({ 350, 95 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Delete Level", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped("Are you sure you want to delete this item?\r\nThis action is irreversable.\r\n");
		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
		if (ImGui::Button("Delete", { 75, 20 })) {
			const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			std::error_code ec;
			if (std::filesystem::remove_all(fullPath, ec))
				m_selected = -1;
			populateLevels(m_subDirectory);
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 100, 20 }))
			ImGui::CloseCurrentPopup();
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}