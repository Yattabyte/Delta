#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/Game/Overlays/LoadingIndicator.h"
#include "Modules/Game/Overlays/Frametime_Counter.h"
#include "Modules/UI/Basic Elements/UI_Element.h"


/** A module responsible for Game Logic. */
class Game_Module final : public Engine_Module {
public:
	// Public Enumerations
	enum class Game_State {
		in_pauseMenu,
		in_game,
	};


	// Public (De)Constructors
	/** Destroy this game module. */
	inline ~Game_Module() noexcept = default;
	/** Construct a game module.
	@param	engine		reference to the engine to use. */
	explicit Game_Module(Engine& engine) noexcept;


	// Public Interface Implementation
	void initialize() noexcept final;
	void deinitialize() noexcept final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime) noexcept;
	/** Retrieve a reference to the currently active ecsWorld in the editor.
	@return					reference to the currently active ecsWorld. */
	ecsWorld& getWorld() noexcept;
	/** Render any and all of the game module's overlays to the screen.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderOverlays(const float& deltaTime) noexcept;
	/** Show the game. */
	void showGame() noexcept;
	/** Either show or hide the pause menu.
	@param	show			whether to show or hide the pause menu. */
	void showPauseMenu(const bool& show) noexcept;


private:
	// Private and deleted
	/** Disallow module move constructor. */
	inline Game_Module(Game_Module&&) noexcept = delete;
	/** Disallow module copy constructor. */
	inline Game_Module(const Game_Module&) noexcept = delete;
	/** Disallow module move assignment. */
	inline const Game_Module& operator =(Game_Module&&) noexcept = delete;
	/** Disallow module copy assignment. */
	inline const Game_Module& operator =(const Game_Module&) noexcept = delete;


	// Private Attributes
	Game_State m_gameState = Game_State::in_game;
	ecsSystemList m_Systems;
	ecsWorld m_world;
	LoadingIndicator m_loadingRing;
	Frametime_Counter m_frameTime;
	std::shared_ptr<UI_Element> m_pauseMenu;
};

#endif // GAME_MODULE_H