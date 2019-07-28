#pragma once
#pragma once
#ifndef PLAYERSPAWNER_SYSTEM_H
#define PLAYERSPAWNER_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"
#include "glm/glm.hpp"


/** A system responsible for spawning players at a specific spawn point. */
class PlayerSpawn_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this free-look system. */
	inline ~PlayerSpawn_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct a free-look system. */
	inline PlayerSpawn_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(PlayerSpawn_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: PlayerSpawn_System");

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		auto & graphicsModule = m_engine->getModule_Graphics();
		for each (const auto & componentParam in components) {
			PlayerSpawn_Component * spawnComponent = (PlayerSpawn_Component*)componentParam[0];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[1];

			auto & transform = transformComponent->m_transform;
			if (m_playerCount == 0ull) {
				auto & world = m_engine->getModule_World();
				Player3D_Component player;
				Transform_Component trans;

				trans.m_transform = transform;

				BaseECSComponent * entityComponents[] = { &player, &trans };
				int types[] = { Player3D_Component::ID, Transform_Component::ID };
				world.makeEntity(entityComponents, types, 2ull);
				m_playerCount++;
			}
		}
	};


private:
	// Private Methods
	/** Clear out the players in this level. */
	inline void clear() {		
		m_playerCount = 0ull;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	size_t m_playerCount = 0ull;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PLAYERSPAWNER_SYSTEM_H