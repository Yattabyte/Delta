#pragma once
#ifndef MUSIC_S_H
#define MUSIC_S_H 

#include "States/Puzzle/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "States/Puzzle/Common_Definitions.h"
#include "Assets/Sound.h"
#include "Engine.h"


/** Responsible for playing music in response to game events. */
class Music_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle music system. */
	inline ~Music_System() = default;
	/** Construct a puzzle music system. */
	inline Music_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);

		// Asset Loading
		m_soundSong = Shared_Sound(m_engine, "Game\\song.wav");
		m_soundSongCrit = Shared_Sound(m_engine, "Game\\song critical.wav");		
	}


	// Public Interface Implementation
	inline virtual bool readyToUse() override {
		return m_soundSong->existsYet() && m_soundSongCrit->existsYet();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		const auto & soundMgr = m_engine->getManager_Sounds();		
		
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];	

			// Exit early if game hasn't started
			if (!board.m_gameInProgress) {
				if (m_songHandle != 0)
					soundMgr.stopWav(m_songHandle);
				continue;
			}

			float BeatSeconds;
			if (!board.m_critical) {
				if (!m_musicPlaying) {
					m_musicPlaying = true;
					m_failPlaying = false;
					soundMgr.stopWav(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSong, 0.75f, true, 3.0);
					board.m_music.accumulator = 0.0f;
				}
				BeatSeconds = (1.0f / 70.0f) * 60.0f;
			}
			else {	
				if (!m_failPlaying) {
					m_failPlaying = true;
					m_musicPlaying = false;
					soundMgr.stopWav(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSongCrit, 0.75f, true);
					board.m_music.accumulator = 0.0f;
				}
				BeatSeconds = (1.0f / 52.5f) * 60.0f;
			}
			float test = std::fmodf(board.m_music.accumulator, BeatSeconds);
			board.m_music.beatSeconds = BeatSeconds;
			board.m_music.beat = bool(test + deltaTime >= BeatSeconds);
			board.m_music.accumulator += deltaTime;
			board.m_data->music.beat = (sinf(((board.m_music.accumulator / BeatSeconds) + 0.875f) * glm::pi<float>()) + 1.0f) / 2.0f;
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundSong, m_soundSongCrit;
	bool m_musicPlaying = false, m_failPlaying = false;
	unsigned int m_songHandle = 0;
};

#endif // MUSIC_S_H