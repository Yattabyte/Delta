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
	inline PauseMenu(Engine * engine)
		: Menu(engine) {
		// Title
		m_title->setText("PAUSE MENU");

		// Add 'Start Game' button
		addButton(engine, "RESUME", [&]() { resume(); });
			
		// Add 'Options' button
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		addButton(engine, "  OPTIONS >", [&]() { goToOptions(); });
		m_optionsMenu->addCallback(OptionsMenu::on_back, [&]() { returnFromOptions(); });

		// Add 'Quit' button
		addButton(engine, "QUIT", [&]() { quit(); });
		
		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_optionsMenu->setScale(scale);
		});

		// Populate Focus Map
		m_focusMap = std::make_shared<FocusMap>();
		m_focusMap->addElement(m_layout);
		m_engine->getModule_UI().setFocusMap(getFocusMap());
	}


protected:
	// Protected Methods
	/** Choose 'resume' from the pause menu. */
	inline void resume() {
		enactCallback(on_resume_game);
	}
	/** Choose 'options' from the main menu. */
	inline void goToOptions() {
		// Transfer appearance and control to options menu
		auto & ui = m_engine->getModule_UI();
		ui.pushRootElement(m_optionsMenu);
		ui.setFocusMap(m_optionsMenu->getFocusMap());
		m_layout->setSelectionIndex(-1);
		enactCallback(on_options);
	}
	/** Chosen when control is returned from the options menu. */
	inline void returnFromOptions() {
		// Transfer control back to this menu
		m_engine->getModule_UI().setFocusMap(getFocusMap());
	}
	/** Choose 'quit' from the pause menu. */
	inline void quit() {
		m_engine->getModule_UI().clear();
		m_engine->shutDown();
		enactCallback(on_quit);
	}


	// Protected Attributes
	std::shared_ptr<OptionsMenu> m_optionsMenu;
};

#endif // PAUSEMENU_H