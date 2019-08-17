#pragma once
#ifndef TRANSLATION_GIZMO_H
#define TRANSLATION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/GL/StaticBuffer.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;

/***/
class Translation_Gizmo {
public:
	// Public (de)Constructors
	/***/
	~Translation_Gizmo();
	/***/
	Translation_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/***/
	bool checkMouseInput(const float& deltaTime);
	/***/
	void render(const float& deltaTime);
	/***/
	void setPosition(const glm::vec3& position);


private:
	// Private Methods
	/***/
	bool rayCastMouse(const float& deltaTime);


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_directions = glm::vec3(1);
	enum SelectedAxes : unsigned int {
		NONE	= 0b0000'0000,
		X_AXIS	= 0b0000'0001,
		Y_AXIS	= 0b0000'0010,
		Z_AXIS	= 0b0000'0100,
	};
	unsigned int m_selectedAxes = NONE;
	glm::vec3 m_startingPosition = glm::vec3(0.0f), m_startingOffset = glm::vec3(0.0f), m_axisDelta = glm::vec3(0.0f);
	std::vector<ecsEntity*> m_translation;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_model;
	Shared_Shader m_gizmoShader;
	StaticBuffer m_indicatorIndirectBuffer;
	BaseECSSystem* m_pickerSystem;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TRANSLATION_GIZMO_H