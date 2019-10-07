#pragma once
#ifndef STARTSCREEN_MODULE_H
#define STARTSCREEN_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A module responsible for the starting screen logic. */
class StartScreen_Module final : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this start screen module. */
	inline ~StartScreen_Module() = default;
	/** Construct a start screen module. */
	inline StartScreen_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;


	// Public Methods
	/***/
	void frameTick(const float& deltaTime);
	/** Display the start menu. */
	void showStartMenu();


private:
	// Private Attributes
	ecsWorld m_world;
	std::shared_ptr<UI_Element> m_startMenu;
};

#endif // STARTSCREEN_MODULE_H