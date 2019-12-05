#pragma once
#ifndef OPTIONS_VIDEO_H
#define OPTIONS_VIDEO_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"
#include <sstream>


/** A UI element serving as a video options menu. */
class Options_Video : public Options_Pane {
public:
	// Public (De)Constructors
	/** Destroy the video pane. */
	inline ~Options_Video() = default;
	/** Construct a video pane.
	@param	engine		reference to the engine to use. */
	explicit Options_Video(Engine& engine) noexcept;


protected:
	// Protected Attributes
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONS_VIDEO_H