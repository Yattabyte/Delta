#pragma once
#ifndef BOARD_S_H
#define BOARD_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"


/** Responsible for updating board visual effects based on the game state. */
class Board_System : public Game_System_Interface {
public:
	// (de)Constructors
	~Board_System() = default;
	Board_System() {
		// Declare component types used
		addComponentType(Board_Component::ID);	
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];	
			board.m_data->data->sysTime = float(glfwGetTime());

			// Exit early if game hasn't started
			if (!board.m_gameStarted)
				continue;

			// Board effects
			board.m_gameTick = ++board.m_gameTick > (TickCount_GameAnimation / (8.0f * board.m_data->data->excitementLinear)) ? 0 : board.m_gameTick;
			board.m_data->data->gameWave = 2.0f * float(board.m_gameTick) / (TickCount_GameAnimation / (8.0f * board.m_data->data->excitementLinear)) - 1.0f;
			board.m_data->data->excitementLinear = std::max(0.0f, std::min(1.0f, board.m_data->data->excitementLinear -= 0.001f));
		}
	}
};

#endif // BOARD_S_H