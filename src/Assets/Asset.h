#pragma once
#ifndef	ASSET_H
#define	ASSET_H

#include <atomic>
#include <functional>
#include <map>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>


class Asset;
class Engine;
using Shared_Asset = std::shared_ptr<Asset>;

/** An abstract base-class for assets.
@brief	Represents some form of data to be loaded from disk, such as shaders, models, levels, and sounds.
@note		is an abstract class instead of interface to reduce redundant code.
@usage	Should be created once, and its pointer passed around using shared pointers. */
class Asset
{
public:
	// (de)Constructors
	/** Destroy the asset only when all references are destroyed. */
	~Asset() = default;
	/** Create asset that uses the specified file-path. */
	Asset(const std::string & filename);


	// Public Methods	
	/** Gets the file name of this asset.
	@return				the file name belonging to this asset */
	std::string getFileName() const;
	/** Sets the file name of this asset.
	@param	filename	the file name to set this asset to */
	void setFileName(const std::string & filename);	
	/** Attaches a callback method to be triggered when the asset finishes loading.
	@param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	@param	callback	the method to be triggered
	@param	<Callback>	the (auto-deduced) signature of the method */
	template <typename Callback>
	void addCallback(void * pointerID, Callback && callback) {
		m_callbacks[pointerID] = std::forward<Callback>(callback);

		if (existsYet())
			callback();
	}
	/** Removes a callback method from triggering when the asset finishes loading.
	@param	pointerID	the pointer to the object owning the callback to be removed */
	void removeCallback(void * pointerID);
	/** Returns whether or not this asset has completed finalizing.
	@return				true if this asset has finished finalizing, false otherwise. */
	bool existsYet() const;


protected:
	// Protected Attributes
	std::atomic_bool m_finalized = false;
	std::string m_filename = "";
	std::map<void*, std::function<void()>> m_callbacks;
	friend class AssetManager;


	// Protected Interface
	/** Initializes the asset. */
	virtual void initialize(Engine * engine, const std::string & relativePath) = 0;
	/** Finalizes the asset. */
	virtual void finalize(Engine * engine);
};

#endif // ASSET_H