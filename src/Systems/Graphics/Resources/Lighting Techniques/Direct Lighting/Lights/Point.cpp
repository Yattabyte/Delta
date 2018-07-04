#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Point.h"
#include "Systems\World\World.h"
#include "Engine.h"
#include <minmax.h>


Point_Tech::~Point_Tech()
{
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
	m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_POINT, this);
}

Point_Tech::Point_Tech(Engine * engine, Light_Buffers * lightBuffers)
{
	// Default Parameters
	m_engine = engine;
	m_lightSSBO = &lightBuffers->m_lightPointSSBO;
	m_size = 0;

	// Asset Loading
	m_engine->createAsset(m_shader_Lighting, string("Base Lights\\Point\\Light"), true);
	m_engine->createAsset(m_shader_CullDynamic, string("Base Lights\\Point\\Culling_Dynamic"), true);
	m_engine->createAsset(m_shader_CullStatic, string("Base Lights\\Point\\Culling_Static"), true);
	m_engine->createAsset(m_shader_ShadowDynamic, string("Base Lights\\Point\\Shadow_Dynamic"), true);
	m_engine->createAsset(m_shader_ShadowStatic, string("Base Lights\\Point\\Shadow_Static"), true);
	m_engine->createAsset(m_shapeSphere, string("sphere"));

	// Primitive Construction
	m_sphereVAOLoaded = false;
	m_sphereVAO = Asset_Primitive::Generate_VAO();
	m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeSphere->addCallback(this, [&]() mutable {
		m_sphereVAOLoaded = true;
		m_shapeSphere->updateVAO(m_sphereVAO);
		const GLuint data = m_shapeSphere->getSize();
		m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});

	// Callbacks
	m_regenSShadows = false;
	m_engine->getSubSystem<System_World>("World")->notifyWhenLoaded(&m_regenSShadows);

	// Initialize Shadows
	m_shadowSize.x = m_engine->addPrefCallback(PreferenceState::C_SHADOW_SIZE_POINT, this, [&](const float &f) {setSize(f); });
	m_shadowSize = vec2(max(1.0f, m_shadowSize.x));
	m_shadowCount = 0;
	for (int x = 0; x < 2; ++x) {
		m_shadowFBO[x] = 0;
		m_shadowDepth[x] = 0;
		m_shadowDistance[x] = 0;
		m_shadowWNormal[x] = 0;
		m_shadowRFlux[x] = 0;
		glCreateFramebuffers(1, &m_shadowFBO[x]);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_shadowDepth[x]);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_shadowDistance[x]);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_shadowWNormal[x]);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_shadowRFlux[x]);

		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDepth[x], GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_DEPTH_ATTACHMENT, m_shadowDepth[x], 0);

		// Create the Shadow depth-distance buffer
		glTextureImage3DEXT(m_shadowDistance[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_R16F, m_shadowSize.x, m_shadowSize.y, 6, 0, GL_RED, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowDistance[x], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDistance[x], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDistance[x], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDistance[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDistance[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT0, m_shadowDistance[x], 0);

		// Create the World Normal buffer
		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, 6, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT1, m_shadowWNormal[x], 0);

		// Create the Radiant Flux buffer
		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, 6, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT2, m_shadowRFlux[x], 0);

		const GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_shadowFBO[x], 3, Buffers);

		const GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO[x], GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Point light Technique", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}
}

vec2 Point_Tech::getSize() const
{
	return m_shadowSize;
}

void Point_Tech::registerShadowCaster(int & array_spot)
{
	if (m_freedShadowSpots.size()) {
		array_spot = m_freedShadowSpots.front();
		m_freedShadowSpots.pop_front();
	}
	else {
		array_spot = m_shadowCount;
		m_shadowCount++;
	}

	// Adjust the layer count every time a new light is added (preserve memory rather than preallocating memory for shadows that don't exist)
	for (int x = 0; x < 2; ++x) {
		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_shadowDistance[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_R16F, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RED, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
	}
}

void Point_Tech::unregisterShadowCaster(int & array_spot)
{
	bool found = false;
	for (int x = 0, size = m_freedShadowSpots.size(); x < size; ++x)
		if (m_freedShadowSpots[x] == array_spot)
			found = true;
	if (!found)
		m_freedShadowSpots.push_back(array_spot);
}

void Point_Tech::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Point");
	if (m_size && m_sphereVAOLoaded) {
		// Retrieve a sorted list of most important lights to run shadow calc for.
		PriorityLightList queue(updateQuality, camPos); 
		m_lightList = vis_token.getTypeList<Lighting_Component>("Light_Point");

		for each (const auto &component in m_lightList)
			queue.insert(component);

		m_queue = queue.toList();
		for each (const auto &c in m_queue)
			c->update(CAM_GEOMETRY_DYNAMIC);

		if (m_regenSShadows)
			for each (const auto &c in m_lightList)
				c->update(CAM_GEOMETRY_STATIC);
	
		vector<GLuint> visArray(m_size);
		unsigned int count = 0;
		for each (const auto &component in m_lightList)
			visArray[count++] = component->getBufferIndex();
		m_visShapes.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size); // update primCount (2nd param)
	}
}

void Point_Tech::updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution)
{
}

void Point_Tech::renderOcclusionCulling()
{
	if (m_size && m_shader_CullDynamic->existsYet()) {
		// Cull dynamic geometry
		m_shader_CullDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[0]);
		glNamedFramebufferDrawBuffer(m_shadowFBO[0], GL_NONE);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (const auto &c in m_queue)
			c->occlusionPass(CAM_GEOMETRY_DYNAMIC);
		if (m_regenSShadows && m_shader_CullStatic->existsYet()) {
			// Cull static geometry
			m_shader_CullStatic->bind();
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[1]);
			glNamedFramebufferDrawBuffer(m_shadowFBO[1], GL_NONE);
			for each (auto & c in m_lightList)
				c->occlusionPass(CAM_GEOMETRY_STATIC);
		}
	}
}

void Point_Tech::renderShadows()
{
	if (m_size && m_shader_ShadowDynamic->existsYet()) {
		// Render dynamic geometry
		m_shader_ShadowDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_shadowFBO[0], 3, Buffers);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[0]);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (auto &component in m_queue)
			component->shadowPass(CAM_GEOMETRY_DYNAMIC);
		if (m_regenSShadows && m_shader_ShadowStatic->existsYet()) {
			m_regenSShadows = false;
			// Render static geometry
			m_shader_ShadowStatic->bind();
			glNamedFramebufferDrawBuffers(m_shadowFBO[1], 3, Buffers);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[1]);
			for each (auto &component in m_lightList)
				component->shadowPass(CAM_GEOMETRY_STATIC);
		}
	}
}

void Point_Tech::renderLightBounce()
{
}

void Point_Tech::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_sphereVAOLoaded) {
		glEnable(GL_STENCIL_TEST);
		glCullFace(GL_FRONT);

		m_shader_Lighting->bind();										// Shader (points)
		glBindTextureUnit(4, m_shadowDistance[0]);						// Shadow maps (dynamic)
		glBindTextureUnit(5, m_shadowDistance[1]);						// Shadow maps (static)
		m_visShapes.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (points)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_sphereVAO);									// Cone VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shader_Lighting->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		m_shader_Lighting->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void Point_Tech::clearShadow(const unsigned int & type, const int & layer)
{
	const float clearDepth(1.0f);
	const vec3 clear(0.0f);
	glClearTexSubImage(m_shadowDepth[type], 0, 0, 0, layer * 6, m_shadowSize.x, m_shadowSize.y, 6.0, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	glClearTexSubImage(m_shadowDistance[type], 0, 0, 0, layer * 6, m_shadowSize.x, m_shadowSize.y, 6.0, GL_RGB, GL_FLOAT, &clearDepth);
	glClearTexSubImage(m_shadowWNormal[type], 0, 0, 0, layer * 6, m_shadowSize.x, m_shadowSize.y, 6.0, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_shadowRFlux[type], 0, 0, 0, layer * 6, m_shadowSize.x, m_shadowSize.y, 6.0, GL_RGB, GL_FLOAT, &clear);
}

void Point_Tech::setSize(const float & size)
{
	m_shadowSize = vec2(max(size, 1));

	for (int x = 0; x < 2; ++x) {
		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_DEPTH_ATTACHMENT, m_shadowDepth[x], 0);

		glTextureImage3DEXT(m_shadowDistance[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_R8, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RED, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT0, m_shadowDistance[x], 0);

		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT1, m_shadowWNormal[x], 0);

		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT2, m_shadowRFlux[x], 0);
		m_regenSShadows = true;
	}
}