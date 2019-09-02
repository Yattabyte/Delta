#pragma once
#ifndef LOADINGINDICATOR_H
#define LOADINGINDICATOR_H

#include "Modules/Game/Overlays/Overlay.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** Graphics effect responsible for showing a loading spinner ring while the engine is loading. */
class LoadingIndicator : public Overlay {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~LoadingIndicator() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline LoadingIndicator(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_texture = Shared_Texture(m_engine, "spinner.png", GL_TEXTURE_2D);
		m_shader = Shared_Shader(m_engine, "Effects\\LoadingIndicator");
		m_shapeQuad = Shared_Auto_Model(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(f, m_renderSize.y)); });
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(m_renderSize.x, f)); });
		resize(m_renderSize);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			m_indirectQuad = IndirectDraw((GLuint)m_shapeQuad->getSize(), 1, 0, 0, GL_CLIENT_STORAGE_BIT);
		});
		
		// World-Changed Callback
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded || state == World_Module::finishLoading)
				m_show = false;
			else if (state == World_Module::startLoading) 
				m_show = true;
			else if (state == World_Module::updated) 
				m_blendAmt = 1.0f;			
		});
	}


	// Public Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_texture->existsYet())
			return;
		if (m_show)
			m_blendAmt += deltaTime;
		else
			m_blendAmt -= deltaTime;
		if (!m_show && m_blendAmt <= 0.0f)
			return;
		m_blendAmt = std::max<float>(0.0f, std::min<float>(1.0f, m_blendAmt));
		if (m_blendAmt > -0.0001f || m_blendAmt < 1.0001f) {
			m_time += deltaTime;
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_texture->bind(0);
			m_shader->bind();
			m_shader->setUniform(1, m_projMatrix);
			m_shader->setUniform(2, glm::translate(glm::mat4(1.0f), glm::vec3(m_renderSize.x - 64, 64, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(32)));
			m_shader->setUniform(3, m_time);
			m_shader->setUniform(4, m_blendAmt);
			m_indirectQuad.drawCall();
			m_shader->Release();
			glDisable(GL_BLEND);
		}
	}


private:
	// Private Methods
	inline void resize(const glm::ivec2 &s) {
		m_renderSize = s;
		m_projMatrix = glm::ortho(0.0f, (float)s.x, 0.0f, (float)s.y);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_texture;
	Shared_Auto_Model m_shapeQuad;
	IndirectDraw m_indirectQuad;
	bool m_show = false;
	float m_time = 0.0f, m_blendAmt = 1.0f;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_projMatrix = glm::mat4(1.0f);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // LOADINGINDICATOR_H