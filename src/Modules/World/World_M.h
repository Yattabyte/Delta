#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules\Engine_Module.h"
#include "Assets\Asset_Level.h"
#include "Utilities\ECS\ECS.h"
#include "Utilities\MappedChar.h"
#include <vector>


/** A module responsible for the world/level. */
class World_Module : public Engine_Module {
public:
	// (de)Constructors
	~World_Module();
	World_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;


	// Public Methods
	/** Loads the world. */
	void loadWorld();
	/** Registers a notification flag to be updated when level loaded.
	@param	notifier	flag to be set when level loaded*/
	void addLevelListener(bool * notifier);
	/** Checks whether the level has finished loading. */
	const bool checkIfLoaded();


private:
	// Process the level
	void processLevel();


	// Private Attributes
	bool m_finishedLoading = false;
	std::vector<bool*> m_notifyees;
	Shared_Asset_Level m_level;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h