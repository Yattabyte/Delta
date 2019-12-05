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
class LoadingIndicator final : public Overlay {
public:
	// Public (De)Constructors
	/** Destroy this overlay. */
	~LoadingIndicator() noexcept;
	/** Construct a loading indicator.
	@param	engine		reference to the engine to use. */
	explicit LoadingIndicator(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void applyEffect(const float& deltaTime) noexcept override final;


private:
	// Private Methods
	/** Resize this indicator. 
	@param	size		the new size to use. */
	void resize(const glm::ivec2& size) noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader;
	Shared_Texture m_texture;
	Shared_Auto_Model m_shapeQuad;
	IndirectDraw<1> m_indirectQuad;
	bool m_show = false;
	float m_time = 0.0f, m_blendAmt = 1.0f;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_projMatrix = glm::mat4(1.0f);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // LOADINGINDICATOR_H