#pragma once
#ifndef IBL_PARALLAX_H
#define IBL_PARALLAX_H

#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Reflector_Tech.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;
class EnginePackage;
class Reflector_Component;


/**
 * A reflection technique that uses run-time generated cubemaps, applied in real-time
 */
class IBL_Parallax_Tech : public Reflector_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~IBL_Parallax_Tech();
	/** Constructor. */
	IBL_Parallax_Tech(EnginePackage * enginePackage);


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass();
	virtual void applyEffect();


private:
	// Private Attributes
	EnginePackage * m_enginePackage;
	bool m_regenCubemap;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
	Shared_Asset_Shader m_shaderEffect;
	Shared_Asset_Primitive m_shapeCube;
	StaticBuffer m_cubeIndirectBuffer, m_visRefUBO;
	vector<Reflector_Component*> m_refList;
	size_t m_size;

};
#endif // IBL_PARALLAX_H
