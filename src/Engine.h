#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#define DESIRED_OGL_VER_MAJOR	4
#define DESIRED_OGL_VER_MINOR	5
#define GLEW_STATIC
constexpr char ENGINE_VERSION[]	= "1.7";

#include "ECS\ecs.h"
#include "Managers\AssetManager.h"
#include "Managers\ModelManager.h"
#include "Managers\MaterialManager.h"
#include "Managers\MessageManager.h"
#include "Modules\Graphics\Graphics_M.h"
#include "Modules\World\World_M.h"
#include "Utilities\ActionState.h"
#include "Utilities\InputBinding.h"
#include "Utilities\PreferenceState.h"
#include "Utilities\MappedChar.h"
#include <string>


class GLFWwindow;
class Engine;

struct Rendering_Context {
	Rendering_Context(Engine * engine);
	GLFWwindow * main, *shared;
};

/** The main game engine object. Encapsulates the entire engine state. */
class Engine {
public:
	// Constructors
	/** Destroys the engine. */
	~Engine();
	/** Zero-initialize the engine. */
	Engine();


	// Public Methods
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick();
	/** Ticks the engine's overall simulation by a frame from a secondary thread. 
	@param	exitObject	object signaling when to close the thread */
	void tickThreaded(std::future<void> exitObj);
	/** Checks if the engine wants to shut down.
	@return	true if engine should shut down */
	bool shouldClose();
	/** Returns the preference-value associated with the supplied preference-ID.
	@param	targetKey	the ID associated with the desired preference-value
	@return				the value associated with the supplied preference-ID */
	float getPreference(const PreferenceState::Preference & targetKey) const {
		return m_PreferenceState.getPreference(targetKey);
	}
	/** Sets the supplied preference-value to the supplied preference-ID.
	@param	targetKey	the ID associated with the supplied preference-value
	@param	targetValue	the value to be set to the supplied preference-ID */
	void setPreference(const PreferenceState::Preference & targetKey, const float & targetValue) {
		m_PreferenceState.setPreference(targetKey, targetValue);
	}
	/** Attaches a callback method to be triggered when the supplied preference updates.
	@param	targetKey	the preference-ID to which this callback will be attached
	@param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	@param	observer	the method to be triggered
	@param	<Observer>	the (auto-deduced) signature of the method 
	@return				optionally returns the preference value held for this target */
	template <typename Observer>
	float const addPrefCallback(const PreferenceState::Preference & targetKey, void * pointerID, Observer && observer) {
		return m_PreferenceState.addPrefCallback(targetKey, pointerID, observer);
	}
	/** Removes a callback method from triggering when a particular preference changes.
	@param	targetKey	the preference key that was listening for changes
	@param	pointerID	the pointer to the object owning the callback to be removed */
	void removePrefCallback(const PreferenceState::Preference & targetKey, void * pointerID) {
		m_PreferenceState.removePrefCallback(targetKey, pointerID);
	}
	/** Forward a message to the message manager.
	@param	input				the message to report */
	void reportMessage(const std::string & input);
	/** Forward an error to the message manager.
	@param	error_number		the error number
	@param	input				the error to report
	@param	additional_input	additional input */
	void reportError(const int & error_number, const std::string & input, const std::string & additional_input = "");
	/** Returns this engine's rendering context. */
	GLFWwindow * getRenderingContext() const;
	/** Returns this engine's entity component system. */
	ECS & getECS() { return m_ecs; }
	/** Returns this engine's action state. */
	ActionState & getActionState() { return m_ActionState; }
	/** Returns this engine's preference state. */
	PreferenceState & getPreferenceState() { return m_PreferenceState; }
	/** Returns this engine's asset manager. */
	AssetManager & getAssetManager() { return m_AssetManager; }
	/** Returns this engine's model manager. */
	ModelManager & getModelManager() { return m_modelManager; }
	/** Returns this engine's material manager. */
	MaterialManager & getMaterialManager() { return m_materialManager; }
	/** Returns this engine's message manager. */
	MessageManager & getMessageManager() { return m_messageManager; }
	/** Returns this engine's graphics module. */
	Graphics_Module & getGraphicsModule() { return m_moduleGraphics; }
	/** Returns this engine's world module. */
	World_Module & getWorldModule() { return m_moduleWorld; }


	// Static Methods
	/** Retrieves the application's running directory.
	@return					std::string of the absolute directory that this executable ran from */
	static std::string Get_Current_Dir();
	/** Check if a given file exists.
	@param	name			the full file path
	@return					true if the file exists, false otherwise */
	static bool File_Exists(const std::string & name);

	
private:
	// Private Methods
	void updateInput(const float & deltaTime);


	// Private Managers
	MessageManager m_messageManager;
	ModelManager m_modelManager;
	MaterialManager m_materialManager;
	AssetManager m_AssetManager;


	// Private Attributes
	float m_lastTime; 
	float m_frameAccumulator;
	int m_frameCount;
	ECS	m_ecs;
	ActionState	m_ActionState;
	InputBinding m_inputBindings;
	PreferenceState	m_PreferenceState;
	Rendering_Context m_renderingContext;


	// Private Modules
	Graphics_Module m_moduleGraphics;
	World_Module m_moduleWorld;
};

#endif // ENGINE_H