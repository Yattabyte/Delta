#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsEntity.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/MappedChar.h"
#include <map>
#include <functional>
#include <vector>


/** A module responsible for the world/level. */
class World_Module : public Engine_Module {
public:
	// Public Enumerations
	const enum WorldState {
		unloaded,
		startLoading,
		finishLoading,
		updated
	};


	// Public (de)Constructors
	/** Destroy this world module. */
	inline ~World_Module() = default;
	/** Construct a world module. */
	inline World_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float& deltaTime) override;


	// Public Methods
	/** Loads the world, specified by the map name.
	@param	mapName				the name of the map to load. */
	void loadWorld(const std::string& mapName);
	/** Saves the world with a specified map name.
	@param	mapName				the name of the map to save as. */
	void saveWorld(const std::string& mapName);
	/** Unload the current world. */
	void unloadWorld();
	/** Serialize a specific set of entities to a char vector.
	@param	entityHandles		the set of entities identified by their handles to serialize.
	@return						char vector containing serialized entity data. */
	std::vector<char> serializeEntities(const std::vector<ecsHandle>& entityHandles);
	/** Serialize a specific entity to a char vector.
	@param	entityHandle		the entity to serialize.
	@return						char vector containing serialized entity data. */
	std::vector<char> serializeEntity(const ecsHandle& entityHandle);
	/** Deserialize an entity from a char array.
	@param	data				previously serialized entity data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far.
	@param	parentHandle		optional handle to parent entity, designed to be called recursively if entity has children.
	@param	desiredHandle		optional specific handle to use, if empty will use handle held in data stream.
	@return						a handle and a pointer pair to the entity created. */
	std::pair<ecsHandle, ecsEntity*> deserializeEntity(const char * data, const size_t& dataSize, size_t& dataRead, const ecsHandle& parentHandle = ecsHandle(), const ecsHandle& desiredHandle = ecsHandle());
	/** Serialize a specific component to a char vector.
	@param	component			the component to serialize.
	@return						char vector containing serialized component data. */
	std::vector<char> serializeComponent(BaseECSComponent* component);
	/** Deserialize a component from a char array.
	@param	data				previously serialized component data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far. */
	std::pair<BaseECSComponent*, int> deserializeComponent(const char * data, const size_t& dataSize, size_t& dataRead);
	/** Registers a notification function to be called when the world state changes.
	@param	alive				a shared pointer indicating whether the caller is still alive or not.
	@param	notifier			function to be called on state change. */
	void addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func);
	/** Generate a universally unique identifier. 
	@return						a new ID. */
	static ecsHandle generateUUID();
	/** Create an entity from a list of input components.
	@param	components			array of component pointers, whom will be hard copied.
	@param	componentIDS		array of component ids.
	@param	numComponents		the number of components in the array.
	@param	name				optional entity name, more for use in the level editor.
	@param	UUID				optional entity UUID, if empty will auto-generate.
	@param	parentUUID			optional parent entity UUID, if not at the level root. */
	ecsHandle makeEntity(BaseECSComponent** components, const int* componentIDS, const size_t& numComponents, const std::string& name = "Entity", const ecsHandle& UUID = ecsHandle(), const ecsHandle& parentUUID = ecsHandle());
	/** Remove an entity.
	@param	entityHandle		handle to the entity to be removed. */
	void removeEntity(const ecsHandle& entityHandle);
	/** Adds a component to an entity.
	@param	entityHandle		handle to the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	inline bool addComponent(const ecsHandle& entityHandle, const BaseECSComponent* component) {
		return addComponentInternal(entityHandle, component->get_id(), component);
	}
	/** Removes a component from an entity.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	<BaseECSComponent>	the category of component being removed.
	@return						true on successful removal, false otherwise. */
	template <typename T>
	inline bool removeComponent(const ecsHandle& entityHandle) {
		return removeComponent(entityHandle, T::ID);
	}
	/** Remove a specific component from an entity and the world.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	componentID			the runtime ID identifying the component class. 
	@return						true on successful removal, false otherwise. */
	inline bool removeComponent(const ecsHandle& entityHandle, const int& componentID) {
		return removeComponentInternal(entityHandle, componentID);
	}
	/** Retrieve a component.
	@param	entityHandle		handle to the entity to retrieve from.
	@param	<BaseECSComponent>	the category of component being retrieved. 
	@return						the specific component of the type requested on success, nullptr otherwise. */
	template <typename T>
	inline T* getComponent(const ecsHandle& entityHandle) {
		return (T*)getComponent(entityHandle, T::ID);
	}
	/** Retrieve a component.
	@param	entityHandle		handle to the entity to retrieve from.
	@param	componentID			the runtime ID identifying the component class.
	@return						the specific component on success, nullptr otherwise. */
	inline BaseECSComponent* getComponent(const ecsHandle& entityHandle, const int& componentID) {
		if (auto * entity = getEntity(entityHandle))
			return getComponentInternal(entity->m_components, m_components[componentID], componentID);
		return nullptr;
	}
	/** Parent an entity to another entity.
	@param	parentEntity		the handle to the desired parent entity.
	@param	childEntity			the handle to the desired child entity. */
	void parentEntity(const ecsHandle& parentEntity, const ecsHandle& childEntity);
	/** Strip a child entity of its parent. 
	@param	childEntity			handle to the child entity, whom will be stripped of its parent. */
	void unparentEntity(const ecsHandle& childEntity);
	/** Try to find a list of entities matching the UUID's provided.
	@param	UUIDs				list of target entity UUID's
	@return						list of pointers to the found entities. Dimensions may not match input list (nullptrs omitted) */
	std::vector<ecsEntity*> getEntities(const std::vector<ecsHandle>& uuids);
	/** Try to find an entity matching the UUID provided.
	@param	UUID				the target entity's UUID.
	@return						pointer to the found entity on success, nullptr on failure. */
	ecsEntity* getEntity(const ecsHandle& uuid);
	/** Retrieve the top-level root of the map.
	@return						a vector of all level entities. */
	std::vector<ecsHandle> getEntityHandles(const ecsHandle& root = ecsHandle());
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList& systems, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem* system, const float& deltaTime);
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	types				list of component types to retrieve.
	@param	flags				list of flags, designating a component as required or optional.
	@param	func				lambda function serving as a system. */
	void updateSystem(const float& deltaTime, const std::vector<int>& types, const std::vector<int>& flags, const std::function<void(const float&, const std::vector<std::vector<BaseECSComponent*>>&)>& func);


private:
	// Private Methods
	/** Notify all world-listeners of a state change.
	@param	state				the new state to notify listeners of. */
	void notifyListeners(const WorldState& state);
	/** Delete a component matching the category ID supplied, at the given index.
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const int& componentID, const int& index);
	/** Adds a component to the entity specified.
	@param	entityHandle		handle to the entity to add the component to.
	@param	componentID			the class ID of the component.
	@param	component			the specific component to add to the entity.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	bool addComponentInternal(const ecsHandle& entityHandle, const int& componentID, const BaseECSComponent* component);
	/** Remove a component from the entity specified.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	componentID			the class ID of the component.
	@return						true on remove success, false otherwise. */
	bool removeComponentInternal(const ecsHandle& entityHandle, const int& componentID);
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	array				the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	BaseECSComponent* getComponentInternal(std::vector<std::pair<int, int>>& entityComponents, std::vector<uint8_t>& array, const int& componentID);
	/** Retrieve the components relevant to an ecs system.
	@param	componentTypes		list of component types to retrieve.
	@param	componentFlags		list of flags, designating a component as required or optional. */
	std::vector<std::vector<BaseECSComponent*>> getRelevantComponents(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags);
	/** Find the least common component.
	@param	componentTypes		the component types.
	@param	componentFlags		the component flags. */
	size_t findLeastCommonComponent(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags);


	// Private Attributes
	bool m_finishedLoading = false;
	std::map<int, std::vector<uint8_t>> m_components;
	std::map<ecsHandle, ecsEntity*> m_entities;
	WorldState m_state = unloaded;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void(const WorldState&)>>> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h