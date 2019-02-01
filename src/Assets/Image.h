#pragma once
#ifndef	IMAGE_H
#define	IMAGE_H

#include "Assets/Asset.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include <any>
#include <optional>


class Engine;
class Image;

/** Public Policy Enumerations. */
const enum Fill_Policy : GLenum {
	Checkered,
	Solid,
};
const enum Resize_Policy : GLenum {
	Nearest,
	Linear,
};

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Image : public std::shared_ptr<Image> {
public:
	Shared_Image() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	specificSize	an optional size to force the image to
	@param	category		the category of image, if available
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Image(Engine * engine, const std::string & filename, const std::optional<glm::ivec2> & specificSize, const bool & threaded = true, const GLenum & policyFill = Fill_Policy::Checkered, const GLenum & policyResize = Resize_Policy::Linear);
};

/** Holds image data, and nothing more. */
class Image : public Asset
{
public:
	/** Destroy the Image. */
	~Image();
	/** Construct the Image. */
	Image(const std::string & filename, const std::optional<glm::ivec2> & specificSize, const GLenum & policyFill, const GLenum & policyResize);


	// Public Attributes
	glm::ivec2 m_size = glm::ivec2(0);
	GLubyte	* m_pixelData = nullptr;
	GLint m_pitch = 0;
	GLuint m_bpp = 0;
	GLenum m_policyFill = Fill_Policy::Checkered;
	GLenum m_policyResize = Resize_Policy::Linear;


private:
	// Private Methods
	/** Fill the image with the desired colors, in accordance with the fill policy.
	@param	primaryColor	the primary color to use
	@param	secondaryColor	the secondary color to use */
	void fill(const glm::uvec4 primaryColor = glm::uvec4(128, 128, 255, 255), const glm::uvec4 secondaryColor = glm::uvec4(0, 0, 0, 255));
	/** Resize the image.
	@param	newSize			the new size to use. */
	void resize(const glm::ivec2 newSize);


	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Image;
};

#endif // IMAGE_H