#pragma once
#ifndef GLOBALILLUMINATION_RH
#define GLOBALILLUMINATION_RH
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GI_LIGHT_BOUNCE_COUNT 2 // Light bounces used
#define GI_TEXTURE_COUNT 4 // 3D textures used

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Systems\World\Camera.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"

class EnginePackage;
class Geometry_FBO;
class Lighting_FBO;
class Shadow_FBO;


 /**
 * Performs primary and secondary light bounces, using the radiance hints technique.
 * Responsible for indirect diffuse lighting.
 * Supports physically based shaders.
 * Supports directional, point, and spot lights.
 **/
class DT_ENGINE_API GlobalIllumination_RH : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~GlobalIllumination_RH();
	/** Constructor. */
	GlobalIllumination_RH(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO, VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Methods
	/** Binds the framebuffer and its render-targets for writing.
	* @param	bounceSpot		which bounce we are performing */
	void bindForWriting(const GLuint & bounceSpot);
	/** Binds the framebuffer and its render-targets for reading.
	* @param	bounceSpot		which bounce we are performing
	* @param	textureUnit		which texture unit we are going to start with (minimum 0) */
	void bindForReading(const GLuint & bounceSpot, const unsigned int & textureUnit);
	/** Bind the noise texture
	* @param	textureUnit		the texture unit to bind the noise texture */
	void bindNoise(const GLuint textureUnit);


	// Private Attributes
	EnginePackage * m_enginePackage;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shadow_FBO * m_shadowFBO;
	// Shared SSBO's
	VectorBuffer<Directional_Struct> * m_lightDirSSBO;
	VectorBuffer<Point_Struct> * m_lightPointSSBO;
	VectorBuffer<Spot_Struct> * m_lightSpotSSBO;
	Shared_Asset_Shader m_shaderDirectional_Bounce, m_shaderPoint_Bounce, m_shaderSpot_Bounce, m_shaderGISecondBounce, m_shaderGIReconstruct;
	Shared_Asset_Primitive m_shapeQuad;
	bool m_vaoLoaded;
	GLuint m_quadVAO, m_bounceVAO;
	GLuint m_fbo[GI_LIGHT_BOUNCE_COUNT]; // 1 fbo per light bounce
	GLuint m_textures[GI_LIGHT_BOUNCE_COUNT][GI_TEXTURE_COUNT]; // 4 textures per light bounce
	GLuint m_noise32;
	float m_nearPlane;
	float m_farPlane;
	ivec2 m_renderSize;
	GLuint m_resolution;
	Camera m_camera;
	StaticBuffer m_attribBuffer, m_Indirect_Slices_Dir, m_Indirect_Slices_Point, m_Indirect_Slices_Spot, m_IndirectSecondLayersBuffer, m_quadIndirectBuffer;
	DynamicBuffer m_visPoints, m_visSpots;
};

#endif // GLOBALILLUMINATION_RH