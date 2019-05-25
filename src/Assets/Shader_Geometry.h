#pragma once
#ifndef	SHADER_GEOMETRY_H
#define	SHADER_GEOMETRY_H

#include "Assets/Asset.h"
#include "Assets/Shader.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <string>


class Engine;
class Shader_Geometry;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader_Geometry : public std::shared_ptr<Shader_Geometry> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader_Geometry() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Shader_Geometry(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** An encapsulation of a vertex/geometry/fragment OpenGL shader program, extending Shader. */
class Shader_Geometry : public Shader {
public:
	// Public (de)Constructors
	/** Destroy the Shader. */
	~Shader_Geometry();
	/** Construct the Shader. */
	Shader_Geometry(Engine * engine, const std::string & filename);

	
	// Public Attributes
	ShaderObj m_geometryShader = ShaderObj(GL_GEOMETRY_SHADER);


protected:
	// Protected Methods
	// Interface Implementation
	virtual bool initShaders(const std::string & relativePath) override;


private:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Shader_Geometry;
};

#endif // SHADER_GEOMETRY_H
