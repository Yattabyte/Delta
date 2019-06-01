#pragma once
#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Engine.h"


/** A UI element serving as a pause menu. */
class PauseMenu : public Menu {
public:
	// Public Interaction Enums
	const enum interact {
		on_resume_game = last_interact_index,
		on_options,
		on_quit,
	};


	// Public (de)Constructors
	/** Destroy the start menu. */
	inline ~PauseMenu() = default;
	/** Construct a start menu. 
	@param	engine		the engine to use. */
	inline PauseMenu(Engine * engine, UI_Element * parent = nullptr)
		: Menu(engine, parent) {
		// Title
		m_title->setText("PAUSE MENU");

		// Add 'Start Game' button
		addButton(engine, "RESUME", [&]() { resume(); });
			
		// Add 'Options' button
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		addButton(engine, "  OPTIONS >", [&]() { options(); });

		// Add 'Quit' button
		addButton(engine, "QUIT", [&]() { quit(); });
		
		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_optionsMenu->setScale(scale);
		});
	}


protected:
	// Protected Methods
	/** Choose 'resume' from the pause menu. */
	inline void resume() {
		enactCallback(on_resume_game);
	}
	/** Choose 'options' from the pause menu. */
	inline void options() {
		// Transfer appearance and control to options menu
		m_engine->getModule_UI().pushRootElement(m_optionsMenu);
		m_layout->setSelectionIndex(-1);
		enactCallback(on_options);
	}
	/** Choose 'quit' from the pause menu. */
	inline void quit() {
		m_engine->getModule_UI().clear();
		m_engine->shutDown();
		enactCallback(on_quit);
	}


	// Protected Attributes
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // PAUSEMENU_H