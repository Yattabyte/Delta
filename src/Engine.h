#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#define DESIRED_OGL_VER_MAJOR	4
#define DESIRED_OGL_VER_MINOR	5
#define GLEW_STATIC
constexpr char ENGINE_VERSION[]	= "0.186";

#include "Assets\Asset.h"
#include "Systems\World\Camera.h"
#include "Systems\Input\ActionState.h"
#include "Systems\Preferences\PreferenceState.h"
#include "Managers\AssetManager.h"
#include "Managers\ModelManager.h"
#include "Managers\MaterialManager.h"
#include "Managers\MessageManager.h"
#include "Utilities\MappedChar.h"
#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>


using namespace std;
class GLFWwindow;
class Camera;
class System;

/**
 * The main game engine object. Encapsulates the entire engine state.
 * The engine is responsible for storing all the system pointers for use through its life.
 **/
class Engine
{
public:
	// Constructors
	/** Destroys the engine. */
	~Engine();
	/** Zero-initialize the engine. */
	Engine();


	// Public Methods
	/** Initializes the engine, and makes this context active for the calling thread.
	 * @return	true if successfully initialized */
	bool initialize();
	/** Shuts down the engine and ceases all threaded activities ASAP. */
	void shutdown();
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick();
	/** Ticks the engine's overall simulation by a frame from a secondary thread. */
	void tickThreaded();
	/** Checks if the engine wants to shut down.
	 * @return	true if engine should shut down */
	bool shouldClose();
	/** Searches for a subsystem with the given name.
	 * @param	c	a const char array name of the desired system to find
	 * @return		the system requested */
	bool findSubSystem(const char * c) { 
		return m_Systems.find(c);
	}
	/** Returns a type-casted subsystem that matches the given name.
	 * @param	c	a const char array name of the desired system to find
	 * @return		true if it can find the system, false otherwise */
	template <typename T> T * getSubSystem(const char * c) {
		return (T*)m_Systems[c];
	}
	/** Returns the preference-value associated with the supplied preference-ID.
	 * @param	targetKey	the ID associated with the desired preference-value
	 * @return				the value associated with the supplied preference-ID */
	float getPreference(const PreferenceState::Preference & targetKey) const {
		return m_PreferenceState.getPreference(targetKey);
	}
	/** Sets the supplied preference-value to the supplied preference-ID.
	 * @param	targetKey	the ID associated with the supplied preference-value
	 * @param	targetValue	the value to be set to the supplied preference-ID */
	void setPreference(const PreferenceState::Preference & targetKey, const float & targetValue) {
		m_PreferenceState.setPreference(targetKey, targetValue);
	}
	/** Attaches a callback method to be triggered when the supplied preference updates.
	 * @param	targetKey	the preference-ID to which this callback will be attached
	 * @param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	 * @param	observer	the method to be triggered
	 * @param	<Observer>	the (auto-deduced) signature of the method 
	 * @return				optionally returns the preference value held for this target */
	template <typename Observer>
	float const addPrefCallback(const PreferenceState::Preference & targetKey, void * pointerID, Observer && observer) {
		return m_PreferenceState.addPrefCallback(targetKey, pointerID, observer);
	}
	/** Removes a callback method from triggering when a particular preference changes.
	 * @param	targetKey	the preference key that was listening for changes
	 * @param	pointerID	the pointer to the object owning the callback to be removed */
	void removePrefCallback(const PreferenceState::Preference & targetKey, void * pointerID) {
		m_PreferenceState.removePrefCallback(targetKey, pointerID);
	}
	/** Creates an asset or uses a cached copy if it has already been created.
	 * @param	sharedAsset		the cointainer to place the asset
	 * @param	args			the rest of the arguments to be used for initialization
	 */
	template <typename SharedAsset, typename... Args>
	void createAsset(SharedAsset & sharedAsset, Args&&... ax) {
		m_AssetManager.create(sharedAsset, forward<Args>(ax)...);
	}
	/** Forward a message to the message manager.
	 * @param	input				the message to report */
	void reportMessage(const string & input);
	/** Forward an error to the message manager.
	 * @param	error_number		the error number
	 * @param	input				the error to report
	 * @param	additional_input	additional input */
	void reportError(const int & error_number, const string & input, const string & additional_input = "");

	// Getters
	/** Returns this engine's rendering context. */
	GLFWwindow * getRenderingContext() { return m_Context_Rendering; }
	/** Returns this engine's main camera. */
	Camera * getCamera() { return m_Camera; }
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


	// Static Methods
	/** Retrieves the application's running directory.
	* @return					string of the absolute directory that this executable ran from */
	static string Get_Current_Dir();
	/** Check if a given file exists.
	* @param	name			the full file path
	* @return					true if the file exists, false otherwise */
	static bool File_Exists(const string & name);


private:
	// Private Attributes
	bool m_Initialized;	
	float m_lastTime; 
	float m_frameAccumulator;
	int m_frameCount;
	GLFWwindow * m_Context_Rendering;
	Camera * m_Camera;
	AssetManager m_AssetManager;
	ActionState	m_ActionState;
	PreferenceState	m_PreferenceState;
	MappedChar<System*>	m_Systems;
	ModelManager m_modelManager;
	MaterialManager m_materialManager;
	MessageManager m_messageManager;
};

/*! \mainpage Project reVision
 * 
 *  \section info_sec Information
 * 
 *  This project is very much a work in progress.
 * 
 *  \section standards_sec Standards/Conventions used
 *  - All member attributes shall be prefixed with 'm_' followed by the rest in camel case, leading with a lowercase character
 *		- vec3 m_currentPosition;
 *	- All member functions are camel cased with a leading lower case character
 *		- void createObject ( ... );
 *	- Static methods are camel cased with a leading upper case character and underscores between words '_'
 *		- static void Load_Asset ( ... );
 *  - All other private member methods or hidden implementations will be lower case and separated by underscores '_'
 *		- void calculate_position_offset( ... );
 *  - Class names are camel cased with a leading upper case character
 * 		- FooBar
 * 		- Entity
 * 		- GeometryBuffer
 *  - However, where fitting, underscores should be used to bring focus to the implementation at play.\n
 * 	  The Left most term should include the newest implementation in the hierarchy
 * 		- Armor_Item
 * 		- Point_Light_Component
 * 		- System_Interface
 *
 * \subsection namespaces Namespaces
 * - Namespaces methods are camel cased with a leading upper case character
 *		- bool FileExistsOnDisk ( ... );
 *
 *  \section dependencies_sec External Dependencies
 * 
 *  - ASSIMP - Model importer: http://assimp.sourceforge.net/
 *  - Bullet - Physics simulator: http://bulletphysics.org/wordpress/
 *  - FreeImage - Texture importer: http://freeimage.sourceforge.net/
 *  - GLEW - OpenGl extension wrangler: http://glew.sourceforge.net/
 *  - GLFW - OpenGL windowing framework: http://www.glfw.org/
 *  - GLM - OpenGL mathematics library: https://glm.g-truc.net/0.9.8/index.html
 */

/*! \page assets Assets
 * \section assets_sec Engine Assets
 * \title Engine Assets
 * This section contains all of the asset types the engine currently supports.\n
 * All assets require doing the following:
 *		- Re-implementing the base asset class
 *		- Define a custom work order class tailored for use in the asset loader (re-implement base work order).
 *		<br>
 *		
 *	They include:
 *		- Asset
 *		- Asset_Collider
 *		- Asset_Config
 *		- Asset_Cubemap
 *		- Asset_Material
 *		- Asset_Model
 *		- Asset_Primitive
 *		- Asset_Shader
 *		- Asset_Shader_Pkg
 *		- Asset_Texture
 */

 /*! \page entities Entities
 * \section ent_sec	Engine Entities
 * This section contains entities and their components.\n
 * There exists only 1 entity class, as all complex entities can be created by adding unique components to them.\n
 * Entities are created by entityCreator classes, controlled by the EntityFactory.\n
 * 
 * Entities implemented so far include:
 *		- Entity (base class)
 *		- SpotLight
 *		- PointLight
 *		- Sun
 *		- Prop
 *		<br>
 *
 * Components implemented so far include:
 *		- Component (base class)
 *		- Geometry_Component (interface)
 *		- Lighting_Component (interface)
 *		- Anim_Model_Component
 *		- Light_Directional_Component
 *		- Light_Point_Component
 *		- Light_Spot_Component
 */

 /*! \page managers Managers
 * \section mgr_sec Engine Managers
 * This section contains Singleton classes.\n
 * Although singletons are frowned upon in OOP, I couldn't think of any scenario in which these \n
 * few systems would ever benefit from being instantiated more than once.\n
 * They include:
 *		- Asset_Manager
 *		- Material_Manager
 *		- Message_Manager
 */

 /*! \page systems Systems
 * \section	sys_sec	Engine Systems
 * This section details systems implemented thus far for the engine.\n
 * The System_Interface details 3 virtual methods all systems inherit: 
 *		- A safe post-context creation initialization function
 *		- A main-loop update function (with delta-time argument)
 *		- A secondary threaded update function (with delta-time argument)
 *		<br>
 *		
 *	***Why 2 update functions?***
 *	The main update function is intended to be used all the essentials, such as rendering and physics.\n
 *	These things are time sensitive, so if anything that needs frequent updating can be offloaded to a second thread, then they can be implemented in the threaded function.\n
 *  For example, visibility calculations are currently offloaded entirely to the second thread.
 *  
 *  The engine currently requires the following base systems:
 *		- System_Graphics
 *		- System_Input
 *		- System_Logic
 *		- System_Preferences
 *		- System_World
 *		<br>
 *	
 *	It is planned to allow swapping out a system of a given category with a different one that implements that system's interface.
 */

 /*! \page utilities Utilities
 * \section util_sec Engine Utilities
 * This section contains some helper tools and objects that don't fit neatly into any other category.\n
 * The rules for this section are pretty slack, however it shouldn't include 1-time use classes that \n
 * could just as easily be nested and void documenting.
 * They include:
 *		- Engine
 *		- File_Reader
 *		- Image_Importer
 *		- Model_Importer
 *		- Transform
 */

#endif // ENGINE_H