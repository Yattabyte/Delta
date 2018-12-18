#pragma once
#ifndef	ASSET_PRIMITIVE_H
#define	ASSET_PRIMITIVE_H

#include "Assets\Asset_Mesh.h"
#include "GL\glad\glad.h"
#include "GLM\glm.hpp"
#include <vector>


class Engine;
class Asset_Primitive;
struct Single_Primitive_Vertex;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Primitive : public std::shared_ptr<Asset_Primitive> {
public:
	Shared_Primitive() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Primitive(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A basic geometric shape to be used in basic visual processing, such as a quad or a sphere. */
class Asset_Primitive : public Asset
{
public:
	/** Destroy the Primitive. */
	~Asset_Primitive();
	/** Construct the Primitive. */
	Asset_Primitive(const std::string & filename);


	// Public Methods
	/** Returns the vertex-count of this object. 
	@return					vertex-count of this object */
	size_t getSize() const;
	
	
	// Public Attributes
	Shared_Mesh m_mesh;
	std::vector<Single_Primitive_Vertex> m_data;
	GLuint m_uboID = 0, m_vaoID = 0;


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Primitive;
};

struct Single_Primitive_Vertex {
	glm::vec3 vertex;
	glm::vec2 uv;
};

#endif // ASSET_PRIMITIVE_H