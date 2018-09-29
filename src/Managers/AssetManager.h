#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Assets\Asset.h"
#include "Utilities\MappedChar.h"
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>


constexpr unsigned int ASSETMANAGER_MAX_THREADS = 8u;
using Asset_Work_Order = std::function<void(void)>;
class Engine;

/** Manages the storage and retrieval of assets. */
class AssetManager {
public:
	// (de)Constructors
	/** Destroy the asset manager. */
	~AssetManager() = default;
	/** Destroy the asset manager. */
	AssetManager(Engine * engine);

	
	// Public Methods
	/** Queries if an asset already exists with the given filename, fetching if true.
	@brief				Searches for and updates the supplied container with the desired asset if it already exists.
	@param	<Asset_T>	the type of asset to query for
	@param	filename	the relative filename (within the project directory) of the asset to search for
	@return				the asset, if found, or null if not */
	template <typename Asset_T>
	inline std::shared_ptr<Asset_T> queryExistingAsset(const std::string & filename, const bool & dontForceFinalize = false) {
		std::shared_lock<std::shared_mutex> read_guard(m_Mutex_Assets);
		for each (const Shared_Asset asset in m_AssetMap[typeid(Asset_T).name()]) 
			if (asset->getFileName() == filename) {
				read_guard.unlock();
				read_guard.release();
				// Asset may be found, but not guaranteed to be finalized
				// Stay here until it is finalized
				if (!dontForceFinalize)
					while (!asset->existsYet()) 
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
				return std::dynamic_pointer_cast<Asset_T>(asset);
			}			
		return std::shared_ptr<Asset_T>();
	}
	/** A template for adding shareable assets to the map.
	@param	asset	the asset to add to the map */
	template <typename Asset_T>
	inline void addShareableAsset(const Asset_T & asset) {
		std::unique_lock<std::shared_mutex> write_guard(m_Mutex_Assets);
		m_AssetMap[typeid(Asset_T).name()].push_back(asset);
	}
	/** Submits an asset for physical creation, and optionally whether to thread it or not. 
	@param	ini			asset initialization function
	@param	threaded	flag to create in a separate thread	*/
	void submitNewWorkOrder(const Asset_Work_Order && ini, const bool & threaded);
	/** Pop's the first work order and completes it. */
	void beginWorkOrder();
	/** Forwards an asset-is-finalized notification request, which will be activated from the main thread. */
	void submitNotifyee(const std::pair<std::shared_ptr<bool>, std::function<void()>> & callBack);
	/* From the main thread, calls all notification calls (for completed asset loading). */
	void notifyObservers();
	/** Returns whether or not this manager has work left.
	 * @return	true if all work is finished, false otherwise. */
	const bool finishedWork();


private:
	// Private Attributes
	Engine * m_engine;
	std::shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;
	std::shared_mutex m_Mutex_Workorders;
	std::deque<Asset_Work_Order> m_Workorders;
	std::shared_mutex m_mutexNofications;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_notifyees;
};

#endif // ASSETMANAGER_H
