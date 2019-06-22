#include "Modules/World/World_M.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"


World_Module::~World_Module()
{
	// Update indicator
	m_aliveIndicator = false;
	unloadWorld();
	removeComponentType("Transform_Component");
}

void World_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: World...");

	// Add New Component Type
	addComponentType("Transform_Component", [](const ParamList & parameters) {
		const auto position = CastAny(parameters, 0, glm::vec3(0.0f));
		const auto orientation = CastAny(parameters, 1, glm::quat(1, 0, 0, 0));
		const auto scale = CastAny(parameters, 2, glm::vec3(1.0f));

		auto * component = new Transform_Component();
		component->m_transform.m_position = position;
		component->m_transform.m_orientation = orientation;
		component->m_transform.m_scale = scale;
		component->m_transform.update();
		return std::make_pair(component->ID, component);
	});
}

void World_Module::frameTick(const float & deltaTime)
{
	auto & assetManager = m_engine->getManager_Assets();
	auto & modelManager = m_engine->getManager_Models();
	auto & materialManager = m_engine->getManager_Materials();

	// Firstly, check and see if the following systems are ready
	if (!assetManager.readyToUse() || !modelManager.readyToUse() || !materialManager.readyToUse())
		return;

	// Signal that the map has finished loading ONCE
	if (m_state == startLoading)
		notifyListeners(finishLoading);
	else if (m_state == finishLoading) {
		// Lastly, check and see if we observed any changes
		if (assetManager.hasChanged() || modelManager.hasChanged() || materialManager.hasChanged())		
			notifyListeners(updated);
	}
}

void World_Module::loadWorld(const std::string & mapName)
{
	// Unload any previous map
	if (m_level && m_level->existsYet())
		unloadWorld();
	m_level = Shared_Level(m_engine, mapName);
	m_level->addCallback(m_aliveIndicator, std::bind(&World_Module::processLevel, this));	
	
	// Signal that a new map is begining to load
	notifyListeners(startLoading);
}

void World_Module::unloadWorld()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		size_t typeSize = BaseECSComponent::getTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freefn((BaseECSComponent*)&it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	for (size_t i = 0; i < m_entities.size(); ++i)
		delete m_entities[i];
	m_entities.clear();
	
	// Signal that the last map has unloaded
	notifyListeners(unloaded);
}

void World_Module::addLevelListener(const std::shared_ptr<bool> & alive, const std::function<void(const WorldState&)> & func)
{
	m_notifyees.push_back(std::make_pair(alive, func));
}

EntityHandle World_Module::makeEntity(BaseECSComponent ** entityComponents, const uint32_t * componentIDs, const size_t & numComponents)
{
	std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* newEntity = new std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >();
	EntityHandle handle = (EntityHandle)newEntity;
	for (size_t i = 0; i < numComponents; ++i) {
		// Check if componentID is actually valid
		if (!BaseECSComponent::isTypeValid(componentIDs[i])) {
			m_engine->getManager_Messages().error("ECS Error: attempted to make an unsupported component, cancelling entity creation...\r\n");
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}

		// Notify component construction-observers
		if (m_constructionNotifyees.find(componentIDs[i]) != m_constructionNotifyees.end())
			for (const auto[alive, func] : m_constructionNotifyees[componentIDs[i]])
				func(entityComponents[i]);
		addComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
	}

	newEntity->first = (uint32_t)m_entities.size();
	m_entities.push_back(newEntity);
	return handle;
}

void World_Module::removeEntity(const EntityHandle & handle)
{
	std::vector<std::pair<uint32_t, uint32_t>> & entity = handleToEntity(handle);
	for (size_t i = 0; i < entity.size(); ++i)
		deleteComponent(entity[i].first, entity[i].second);

	const uint32_t destIndex = handleToEntityIndex(handle);
	const uint32_t srcIndex = (uint32_t)m_entities.size() - 1u;
	delete m_entities[destIndex];
	m_entities[destIndex] = m_entities[srcIndex];
	m_entities[destIndex]->first = destIndex;
	m_entities.pop_back();
}

void World_Module::addComponentType(const char * name, const std::function<std::pair<uint32_t,BaseECSComponent*>(const ParamList &)> & func)
{
	m_constructorMap[name] = func;
}

void World_Module::removeComponentType(const char * name)
{
	m_constructorMap.erase(name);
}

void World_Module::addNotifyOnComponentType(const uint32_t & ID, const std::shared_ptr<bool> & alive, const std::function<void(BaseECSComponent*)>& func)
{
	m_constructionNotifyees[ID].push_back(std::make_pair(alive, func));
}

void World_Module::updateSystems(ECSSystemList & systems, const float & deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i)
		updateSystem(systems[i], deltaTime);
}

void World_Module::updateSystem(BaseECSSystem * system, const float & deltaTime)
{
	if (auto components = getRelevantComponents(system->getComponentTypes(), system->getComponentFlags()); components.size() > 0ull)
		system->updateComponents(deltaTime, components);
}

void World_Module::updateSystem(const float & deltaTime, const std::vector<uint32_t> & types, const std::vector<uint32_t> & flags, const std::function<void(const float&, const std::vector<std::vector<BaseECSComponent*>>&)>& func)
{
	if (auto components = getRelevantComponents(types, flags); components.size() > 0ull)
		func(deltaTime, components);	
}

void World_Module::processLevel()
{
	if (m_level->existsYet()) {
		std::vector<BaseECSComponent*> components; // holds each entity's components
		std::vector<unsigned int> ids; // holds each entity's component id's
		const char * type;
		for each (auto & lvlEntity in m_level->m_entities) {
			for each (const auto & lvlComponent in lvlEntity.components) {
				// Get component type
				type = lvlComponent.type.c_str();
				// Call the appropriate creator if available
				if (m_constructorMap.find(type)) {
					const auto [ID, component] = m_constructorMap[type](lvlComponent.parameters);
					if (component != nullptr) {
						components.push_back(component);
						ids.push_back(ID);
					}
				}
			}
			// Make an entity out of the available components
			if (components.size())
				makeEntity(components.data(), ids.data(), components.size());
			// Delete temporary components and reset for next entity
			for each (auto * component in components)
				delete component;
			components.clear();
			ids.clear();
		}
	}
}

void World_Module::notifyListeners(const WorldState & state)
{
	// Get all callback functions, and call them if their owners are still alive
	for (int x = 0; x < (int)m_notifyees.size(); ++x) {
		const auto &[alive, func] = m_notifyees[x];
		if (alive && *(alive.get()) == true)
			func(state);
		else {
			m_notifyees.erase(m_notifyees.begin() + x);
			x--;
		}
	}
	m_state = state;
}

void World_Module::deleteComponent(const uint32_t & componentID, const uint32_t & index)
{
	std::vector<uint8_t> mem_array = m_components[componentID];
	ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(componentID);
	const size_t typeSize = BaseECSComponent::getTypeSize(componentID);
	const size_t srcIndex = mem_array.size() - typeSize;

	BaseECSComponent * srcComponent = (BaseECSComponent*)&mem_array[srcIndex];
	BaseECSComponent * destComponent = (BaseECSComponent*)&mem_array[index];
	freefn(destComponent);

	if (index == (uint32_t)srcIndex) {
		mem_array.resize(srcIndex);
		return;
	}
	std::memcpy(destComponent, srcComponent, typeSize);

	// Update references
	std::vector<std::pair<uint32_t, uint32_t>> & srcComponents = handleToEntity(srcComponent->entity);
	for (size_t i = 0; i < srcComponents.size(); ++i) {
		if (componentID == srcComponents[i].first && (uint32_t)srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}
	mem_array.resize(srcIndex);
}

void World_Module::addComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>>& entity, const uint32_t & componentID, BaseECSComponent * component)
{
	ECSComponentCreateFunction createfn = BaseECSComponent::getTypeCreateFunction(componentID);
	std::pair<uint32_t, uint32_t> newPair;
	newPair.first = componentID;
	newPair.second = createfn(m_components[componentID], handle, component);
	entity.push_back(newPair);
}

bool World_Module::removeComponentInternal(EntityHandle handle, const uint32_t & componentID)
{
	std::vector<std::pair<uint32_t, uint32_t>> & entityComponents = handleToEntity(handle);
	for (size_t i = 0; i < entityComponents.size(); ++i) {
		if (componentID == entityComponents[i].first) {
			deleteComponent(entityComponents[i].first, entityComponents[i].second);
			const size_t srcIndex = entityComponents.size() - 1;
			const size_t destIndex = i;
			entityComponents[destIndex] = entityComponents[srcIndex];
			entityComponents.pop_back();
			return true;
		}
	}
	return false;
}

BaseECSComponent * World_Module::getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>> & entityComponents, std::vector<uint8_t> & mem_array, const uint32_t & componentID)
{
	for (size_t i = 0; i < entityComponents.size(); ++i)
		if (componentID == entityComponents[i].first)
			return (BaseECSComponent*)&mem_array[entityComponents[i].second];
	return nullptr;
}

std::vector<std::vector<BaseECSComponent*>> World_Module::getRelevantComponents(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t>& componentFlags)
{
	std::vector< std::vector<BaseECSComponent*> > components;
	if (componentTypes.size() > 0ull) {
		if (componentTypes.size() == 1u) {
			// Super simple procedure for system with 1 component type
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
			const std::vector<uint8_t>& mem_array = m_components[componentTypes[0]];
			components.resize(mem_array.size() / typeSize);
			for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k)
				components[k].push_back((BaseECSComponent*)&mem_array[j]);
		}
		else {
			// More complex procedure for system with > 1 component type
			std::vector<BaseECSComponent*> componentParam(componentTypes.size());
			std::vector<std::vector<uint8_t>*> componentArrays(componentTypes.size());
			for (size_t i = 0; i < componentTypes.size(); ++i)
				componentArrays[i] = &m_components[componentTypes[i]];

			const size_t minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
			const std::vector<uint8_t> & mem_array = *componentArrays[minSizeIndex];
			components.reserve(mem_array.size() / typeSize); // reserve, not resize, as we component at [i] may be invalid
			for (size_t i = 0; i < mem_array.size(); i += typeSize) {
				componentParam[minSizeIndex] = (BaseECSComponent*)&mem_array[i];
				std::vector<std::pair<uint32_t, uint32_t> > & entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);

				bool isValid = true;
				for (size_t j = 0; j < componentTypes.size(); ++j) {
					if (j == minSizeIndex)
						continue;
					componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
					if ((componentParam[j] == nullptr) && (componentFlags[j] & BaseECSSystem::FLAG_REQUIRED)) {
						isValid = false;
						break;
					}
				}
				if (isValid)
					components.push_back(componentParam);
			}
		}
	}
	return components;
}

size_t World_Module::findLeastCommonComponent(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t> & componentFlags)
{
	size_t minSize = (size_t)-1;
	size_t minIndex = (size_t)-1;
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
			continue;
		const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
		const size_t size = m_components[componentTypes[i]].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}
