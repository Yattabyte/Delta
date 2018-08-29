#pragma once
#ifndef	ASSET_MODEL_H
#define	ASSET_MODEL_H
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#include "Assets\Asset.h"
#include "Assets\Asset_Material.h"
#include "Managers\ModelManager.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class ModelManager;
class Asset_Model;
typedef std::shared_ptr<Asset_Model> Shared_Asset_Model;

/** A 3D geometric mesh meant to be used in 3D rendering. */
class Asset_Model : public Asset
{
public:	
	/** Destroy the Model. */
	~Asset_Model();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Model Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Returns the material ID for a skin given an index into this list
	@note					Clamps to the skin list size, so it won't go out of bounds
	@param	index			into this model's skin list. 
	@return					index into the master material list in which this skin can be found at */	
	GLuint getSkinID(const unsigned int & desired);


	// Public Attributes
	std::vector<Shared_Asset_Material>	m_skins;
	GeometryInfo						m_data;
	std::vector<BoneTransform>			m_boneTransforms;
	std::map<std::string, size_t>		m_boneMap;
	std::vector<Animation>				m_animations;
	Node								*m_rootNode;
	glm::vec3							m_bboxMin, m_bboxMax, m_bboxCenter;
	float								m_radius;
	size_t								m_offset, m_count;
	ModelManager						*m_modelManager;


private:
	// Private Constructors
	/** Construct the Model. */
	Asset_Model(const std::string & filename, ModelManager & modelManager);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory);
	virtual void finalize(Engine * engine);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MODEL_H