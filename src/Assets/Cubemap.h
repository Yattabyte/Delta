#pragma once
#ifndef	CUBEMAP_H
#define	CUBEMAP_H

#include "Assets/Image.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"


class Engine;
class Cubemap;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Cubemap : public std::shared_ptr<Cubemap> {
public:
	Shared_Cubemap() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Cubemap(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** Represents an OpenGL cubemap texture object. */
class Cubemap : public Asset
{
public:
	/** Destroy the Cubemap. */
	~Cubemap();
	/** Construct the Cubemap. */
	Cubemap(const std::string & filename);


	// Public Methods
	/** Makes this texture active at a specific texture unit.
	@param	texture_unit	the desired texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);
	
	
	// Public Attributes
	GLuint m_glTexID = 0, m_pboIDs[6] = {0,0,0,0,0,0};
	Shared_Image m_images[6];


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Cubemap;
};
#endif // CUBEMAP_H
