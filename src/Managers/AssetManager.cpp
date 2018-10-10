#include "Managers\AssetManager.h"

 
AssetManager::AssetManager(Engine * engine) : m_engine(engine) {}

void AssetManager::submitNewWorkOrder(const Asset_Work_Order && ini, const bool & threaded) {
	if (threaded) {
		std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		m_Workorders.push_back(std::move(ini));
	}
	else
		ini();
}

void AssetManager::beginWorkOrder()
{
	// Start reading work orders
	std::unique_lock<std::shared_mutex> writeGuard(m_Mutex_Workorders);
	if (m_Workorders.size()) {
		// Remove front of queue
		const Asset_Work_Order workOrder = m_Workorders.front();
		m_Workorders.pop_front();
		writeGuard.unlock();
		writeGuard.release();

		// Initialize asset
		workOrder();
	}
}

void AssetManager::submitNotifyee(const std::pair<std::shared_ptr<bool>, std::function<void()>> & callBack)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
	m_notifyees.push_back(callBack);
	m_changed = true;
}

void AssetManager::notifyObservers()
{
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> copyNotifyees;
	{
		std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
		copyNotifyees = m_notifyees;
		m_notifyees.clear();
	}
	for each (const auto & pair in copyNotifyees)
		if (pair.first)
			pair.second();

}

const bool AssetManager::readyToUse()
{
	std::shared_lock<std::shared_mutex> readGuard(m_Mutex_Workorders);
	return !bool(m_Workorders.size() + m_Workorders.size());
}

const bool AssetManager::hasChanged()
{
	// Changes every time assets finalize, when this manager notifies the assets' observers.
	std::shared_lock<std::shared_mutex> readGuard(m_mutexNofications);
	bool state = m_changed;
	m_changed = false;
	return state;
}
