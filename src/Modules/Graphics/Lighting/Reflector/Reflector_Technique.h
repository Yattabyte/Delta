#pragma once
#ifndef REFLECTOR_TECHNIQUE_H
#define REFLECTOR_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorScheduler_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"


/** A core lighting technique responsible for all parallax reflectors. */
class Reflector_Technique final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this reflector technique. */
	inline ~Reflector_Technique() noexcept {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct a reflector technique.
	@param	engine			reference to the engine to use. 
	@param	sceneCameras	reference to the scene cameras to use. */
	inline Reflector_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) noexcept :
		Graphics_Technique(Technique_Category::PRIMARY_LIGHTING),
		m_engine(engine),
		m_shaderLighting(Shared_Shader(engine, "Core\\Reflector\\IBL_Parallax")),
		m_shaderStencil(Shared_Shader(engine, "Core\\Reflector\\Stencil")),
		m_shaderCopy(Shared_Shader(engine, "Core\\Reflector\\2D_To_Cubemap")),
		m_shaderConvolute(Shared_Shader(engine, "Core\\Reflector\\Cube_Convolution")),
		m_shapeCube(Shared_Auto_Model(engine, "cube")),
		m_shapeQuad(Shared_Auto_Model(engine, "quad")),
		m_sceneCameras(sceneCameras)
	{
		// Auxiliary Systems
		m_auxilliarySystems.makeSystem<ReflectorScheduler_System>(engine, m_frameData);
		m_auxilliarySystems.makeSystem<ReflectorVisibility_System>(m_frameData);
		m_auxilliarySystems.makeSystem<ReflectorSync_System>(m_frameData);

		// Preferences
		auto& preferences = engine.getPreferenceState();
		preferences.getOrSetValue(PreferenceState::Preference::C_ENVMAP_SIZE, m_frameData.envmapSize.x);
		preferences.addCallback(PreferenceState::Preference::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float& f) {
			m_frameData.envmapSize = glm::ivec2(std::max(1u, (unsigned int)f));
			m_viewport->resize(glm::ivec2(m_frameData.envmapSize), (int)m_frameData.reflectorLayers);
			});
		// Environment Map
		m_frameData.envmapSize = glm::ivec2(std::max(1u, (unsigned int)m_frameData.envmapSize.x));
		m_viewport = std::make_shared<Viewport>(glm::ivec2(0), m_frameData.envmapSize, engine);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 };
			m_indirectQuad = StaticMultiBuffer(sizeof(GLuint) * 4, quadData);
			m_indirectQuadConvolute = StaticMultiBuffer(sizeof(GLuint) * 4, quadData);
			});
	}


	// Public Interface Implementations
	inline virtual void clearCache(const float& deltaTime) noexcept override final {
		m_frameData.lightBuffer.endReading();
		m_frameData.viewInfo.clear();
		m_frameData.reflectorsToUpdate.clear();
		m_drawIndex = 0;
		m_drawData.clear();
	}
	inline virtual void updateCache(const float& deltaTime, ecsWorld& world) noexcept override final {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData.viewInfo.resize(m_sceneCameras.size());
		world.updateSystems(m_auxilliarySystems, deltaTime);
	}
	inline virtual void updatePass(const float& deltaTime) noexcept override final {
		// Exit Early
		if (m_enabled && m_shapeQuad->existsYet() && m_shaderCopy->existsYet() && m_shaderConvolute->existsYet())
			updateReflectors(deltaTime);
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final {
		// Exit Early
		if (m_enabled && m_frameData.viewInfo.size() && m_shapeCube->existsYet() && m_shaderLighting->existsYet() && m_shaderStencil->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<GLint> lightIndices;
			for (auto& [camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData.viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				lightIndices.insert(lightIndices.end(), m_frameData.viewInfo[camIndex].lightIndices.begin(), m_frameData.viewInfo[camIndex].lightIndices.end());
			}

			if (lightIndices.size()) {
				auto& drawBuffer = m_drawData[m_drawIndex];
				auto& camBufferIndex = drawBuffer.bufferCamIndex;
				auto& lightBufferIndex = drawBuffer.visLights;

				// Write accumulated data
				camBufferIndex.beginWriting();
				lightBufferIndex.beginWriting();
				drawBuffer.indirectShape.beginWriting();
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
				const GLuint dataShape[] = { (GLuint)m_shapeCube->getSize(), (GLuint)lightIndices.size(), 0,0 };
				drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);
				camBufferIndex.endWriting();
				lightBufferIndex.endWriting();
				drawBuffer.indirectShape.endWriting();

				// Render lights
				renderReflectors(deltaTime, viewport);
				camBufferIndex.endReading();
				lightBufferIndex.endReading();
				drawBuffer.indirectShape.endReading();

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame. */
	inline void updateReflectors(const float& deltaTime) noexcept {
		auto clientTime = m_engine.getTime();
		if (m_frameData.reflectorsToUpdate.size()) {
			m_viewport->resize(m_frameData.envmapSize, (int)m_frameData.reflectorLayers);
			m_viewport->bind();
			m_viewport->clear();

			// Accumulate Perspective Data
			std::vector<std::pair<int, int>> perspectives;
			perspectives.reserve(m_frameData.reflectorsToUpdate.size());
			for (auto& [importance, time, reflectorSpot, camera] : m_frameData.reflectorsToUpdate) {
				// Accumulate all visibility info for the cameras passed in
				int visibilityIndex = 0;
				bool found = false;
				for (int y = 0; y < m_sceneCameras.size(); ++y)
					if (m_sceneCameras.at(y) == camera) {
						visibilityIndex = y;
						found = true;
						break;
					}
				if (found)
					perspectives.push_back({ visibilityIndex, reflectorSpot });
				*time = clientTime;
				camera->setEnabled(false);
			}
			m_indirectQuad.beginWriting();
			m_indirectQuadConvolute.beginWriting();
			const auto instanceCount = (GLuint)perspectives.size(), convoluteCount = (GLuint)m_frameData.reflectorLayers;
			m_indirectQuad.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
			m_indirectQuadConvolute.write(sizeof(GLuint), sizeof(GLuint), &convoluteCount);
			m_indirectQuad.endWriting();
			m_indirectQuadConvolute.endWriting();

			// Update all reflectors at once
			constexpr unsigned int categories = (unsigned int)Graphics_Technique::Technique_Category::GEOMETRY | (unsigned int)Graphics_Technique::Technique_Category::PRIMARY_LIGHTING | (unsigned int)Graphics_Technique::Technique_Category::SECONDARY_LIGHTING;
			m_engine.getModule_Graphics().getPipeline()->render(deltaTime, m_viewport, perspectives, categories);

			// Copy all lighting results into cube faces, generating cubemap's
			m_viewport->m_gfxFBOS.bindForReading("LIGHTING", 0);
			m_frameData.envmapFBO.bindForWriting(0);
			m_shaderCopy->bind();
			m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(GL_STENCIL_TEST);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			m_indirectQuad.endReading();

			// Convolute all completed cubemap's, not just what was done this frame
			m_shaderConvolute->bind();
			m_indirectQuadConvolute.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_frameData.envmapFBO.bindForReading(0);
			for (unsigned int r = 1; r < 6; ++r) {
				// Ensure we are writing to MIP level r
				const unsigned int write_size = (unsigned int)std::max(1.0f, (floor((float)m_frameData.envmapSize.x / pow(2.0f, (float)r))));
				glViewport(0, 0, write_size, write_size);
				m_shaderConvolute->setUniform(1, (float)r / 5.0f);
				m_frameData.envmapFBO.bindForWriting(r);

				// Convolute the 6 faces for this roughness level
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			m_indirectQuadConvolute.endReading();
			Shader::Release();
			m_frameData.reflectorsToUpdate.clear();
		}
	}
	/** Render all the lights
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderReflectors(const float& deltaTime, const std::shared_ptr<Viewport>& viewport) noexcept {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilMask(0xFF);

		// Draw only into depth-stencil buffer
		m_shaderStencil->bind();										// Shader (reflector)
		viewport->m_gfxFBOS.bindForWriting("REFLECTION");				// Ensure writing to reflection FBO
		viewport->m_gfxFBOS.bindForReading("GEOMETRY", 0);				// Read from Geometry FBO
		m_frameData.envmapFBO.bindForReading(4);						// Reflection map (environment texture)
		m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
		m_frameData.lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
		m_drawData[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
		glBindVertexArray(m_shapeCube->m_vaoID);						// Quad VAO
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glFrontFace(GL_CW);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glFrontFace(GL_CCW);

		// Now draw into color buffers
		m_shaderLighting->bind();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glClear(GL_STENCIL_BUFFER_BIT);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_STENCIL_TEST);
		Shader::Release();
	}


	// Private Attributes
	Engine& m_engine;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	Shared_Auto_Model m_shapeCube, m_shapeQuad;
	StaticMultiBuffer<> m_indirectQuad = StaticMultiBuffer(sizeof(GLuint) * 4), m_indirectQuadConvolute = StaticMultiBuffer(sizeof(GLuint) * 4);
	std::shared_ptr<Viewport> m_viewport;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex;
		DynamicBuffer<> visLights;
		StaticMultiBuffer<> indirectShape = StaticMultiBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;


	// Shared Attributes
	ReflectorData m_frameData;
	std::vector<Camera*>& m_sceneCameras;
};

#endif // REFLECTOR_TECHNIQUE_H
