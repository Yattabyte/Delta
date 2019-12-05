#include "Modules/ECS/ecsSystem.h"


std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>> ecsBaseSystem::getComponentTypes() const 
{
	return m_componentTypes;
}

bool ecsBaseSystem::isValid() const noexcept 
{
	for (const auto& [componentID, componentFlag] : m_componentTypes)
		if (((unsigned int)componentFlag & (unsigned int)RequirementsFlag::FLAG_OPTIONAL) == 0)
			return true;
	return false;
}

void ecsBaseSystem::addComponentType(const ComponentID& componentType, const RequirementsFlag& componentFlag) noexcept 
{
	m_componentTypes.push_back({ componentType, componentFlag });
}

bool ecsSystemList::addSystem(const std::shared_ptr<ecsBaseSystem>& system) noexcept 
{
	if (system->isValid()) {
		m_systems.push_back(system);
		return true;
	}
	return false;
}

bool ecsSystemList::removeSystem(const std::shared_ptr<ecsBaseSystem>& system) noexcept 
{
	for (size_t i = 0; i < m_systems.size(); ++i)
		if (system.get() == m_systems[i].get()) {
			m_systems.erase(m_systems.begin() + i);
			return true;
		}
	return false;
}

size_t ecsSystemList::size() const noexcept 
{
	return m_systems.size();
}

std::shared_ptr<ecsBaseSystem> ecsSystemList::operator[](const size_t& index) const noexcept 
{
	return m_systems[index];
}

std::vector<std::shared_ptr<ecsBaseSystem>>::iterator ecsSystemList::begin() noexcept 
{
	return m_systems.begin();
}

std::vector<std::shared_ptr<ecsBaseSystem>>::const_iterator ecsSystemList::begin() const noexcept
{
	return m_systems.cbegin();
}

std::vector<std::shared_ptr<ecsBaseSystem>>::iterator ecsSystemList::end() noexcept
{
	return m_systems.end();
}

std::vector<std::shared_ptr<ecsBaseSystem>>::const_iterator ecsSystemList::end() const noexcept
{
	return m_systems.cend();
}