/*
	Config Manager

	- Loads an engine-wide configuration that other aspects of the engine may need to use
		- For example: Resolution, quality preferences, and key binds
	- Mostly a wrapper for the config asset, but this keeps a static one for all to use
*/

#include "Assets\Asset_Config.h"

#pragma once
#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

using namespace std;

namespace CFG
{
	// Shutsdown by saving the current configuration map to disk
	DELTA_CORE_API void shutdown();
	// Loads a configuration file from disk. Defaults to "config"
	DELTA_CORE_API void loadConfiguration();
	// Saves a configuration file to disk. Defaults to "config"
	DELTA_CORE_API void saveConfiguration();
	// Returns a preference value given a preference key
	DELTA_CORE_API float getPreference(const int &cfg_key);
	// Set a preference at @cfg_key with the value at @value
	DELTA_CORE_API void setPreference(const int &cfg_key, const float &cfg_value);
	// Add a callback function @function to be called whenever the preference @cfg_key changes
	DELTA_CORE_API void addPreferenceCallback(const int &cfg_key, void(*function)());
}

#endif // CONFIG_MANAGER