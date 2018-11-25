#pragma once
#ifndef RENDERING_S_H
#define RENDERING_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Model.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


constexpr unsigned int tileSize = 128u;

/***/
class Rendering_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Rendering_System() = default;
	Rendering_System(Engine * engine) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);

		m_vaoModels = &engine->getModelManager().getVAO();

		// For rendering tiles to the board
		m_orthoProjField = glm::ortho<float>(0, tileSize * 6, 0, tileSize * 12, -1, 1);
		m_orthoProjHeader = glm::ortho<float>(-384, 384, -96, 96, -1, 1);

		// Board FBO & Texture Creation
		glCreateFramebuffers(1, &m_fboIDField);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_boardTexID);
		glTextureImage2DEXT(m_boardTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 12, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_boardTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glNamedFramebufferTexture(m_fboIDField, GL_COLOR_ATTACHMENT0, m_boardTexID, 0);
		glNamedFramebufferDrawBuffer(m_fboIDField, GL_COLOR_ATTACHMENT0);

		glCreateFramebuffers(1, &m_fboIDBars);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_scoreTexID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_timeTexID);
		glTextureImage2DEXT(m_scoreTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 1.5, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_timeTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 1.5, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboIDBars, GL_COLOR_ATTACHMENT0, m_scoreTexID, 0);
		glNamedFramebufferTexture(m_fboIDBars, GL_COLOR_ATTACHMENT1, m_timeTexID, 0);
		const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_fboIDBars, 2, drawBuffers);

		// Error Reporting
		GLenum Status = glCheckNamedFramebufferStatus(m_fboIDField, GL_FRAMEBUFFER);
		//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		//	engine->getMessageManager().error("Game Board Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));		
		if (!glIsTexture(m_boardTexID))
			engine->getMessageManager().error("Game Board Texture is incomplete.");
		Status = glCheckNamedFramebufferStatus(m_fboIDBars, GL_FRAMEBUFFER);
		//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		//	engine->getMessageManager().error("Game Header Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_scoreTexID))
			engine->getMessageManager().error("Game Header Texture is incomplete.");

		// Asset Loading
		m_modelBoard = Asset_Model::Create(engine, "Game\\boardBorder.obj");
		m_modelField = Asset_Model::Create(engine, "Game\\boardField.obj");
		m_modelHeader = Asset_Model::Create(engine, "Game\\boardTop.obj");
		m_modelFooter = Asset_Model::Create(engine, "Game\\boardBottom.obj");
		m_shaderBoard = Asset_Shader::Create(engine, "Game\\Board", true);
		m_shaderTiles = Asset_Shader::Create(engine, "Game\\Tiles", true);
		m_shaderTileScored = Asset_Shader::Create(engine, "Game\\TileScored", true);
		m_shaderScore = Asset_Shader::Create(engine, "Game\\Score", true);
		m_shaderMultiplier = Asset_Shader::Create(engine, "Game\\Multiplier", true);
		m_shaderTimer = Asset_Shader::Create(engine, "Game\\Timer", true);
		m_textureTile = Asset_Texture::Create(engine, "Game\\tile.png");
		m_textureTileScored = Asset_Texture::Create(engine, "Game\\newTileScored.png");
		m_textureTilePlayer = Asset_Texture::Create(engine, "Game\\player.png");
		m_textureScoreNums = Asset_Texture::Create(engine, "Game\\scoreNums.png");
		m_textureTimeStop = Asset_Texture::Create(engine, "Game\\timestop.png");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint & quadSize = (GLuint)m_shapeQuad->getSize();
			// count, primCount, first, reserved
			const GLuint tileData[4] = { quadSize, (12 * 6) + 2, 0, 0 };
			m_bufferIndirectTiles = StaticBuffer(sizeof(GLuint) * 4, tileData, 0);
			const GLuint tileScoreData[4] = { quadSize, 0, 0, 0 };
			m_bufferIndirectTilesScored = StaticBuffer(sizeof(GLuint) * 4, tileScoreData, GL_DYNAMIC_STORAGE_BIT);
		
			const GLuint scoreData[4] = { quadSize, 1, 0, 0 };
			m_bufferIndirectScore = StaticBuffer(sizeof(GLuint) * 4, scoreData);
			const GLuint stopData[4] = { quadSize, 6, 0, 0 };
			m_bufferIndirectStop = StaticBuffer(sizeof(GLuint) * 4, stopData, 0);
			const GLuint multiplierData[4] = { quadSize, 4, 0, 0 };
			m_bufferIndirectMultiplier = StaticBuffer(sizeof(GLuint) * 4, multiplierData, 0);
		});
		m_bufferIndirectBoard = StaticBuffer(sizeof(GLint) * 16, 0, GL_DYNAMIC_STORAGE_BIT);
		m_modelBoard->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelBoard->m_count, 1, m_modelBoard->m_offset, 1 };
			m_bufferIndirectBoard.write(0, sizeof(GLint) * 4, data);
		});
		m_modelField->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelField->m_count, 1, m_modelField->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 4, sizeof(GLint) * 4, data);
		});
		m_modelHeader->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelHeader->m_count, 1, m_modelHeader->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 8, sizeof(GLint) * 4, data);
		});
		m_modelFooter->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelFooter->m_count, 1, m_modelFooter->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 12, sizeof(GLint) * 4, data);
		});
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		if (!areAssetsReady())
			return;

		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			// Update Rendering Data
			constexpr int decimalPlaces[8] = { 10000000,1000000,100000,10000,1000,100,10,1 };
			GLuint scoreLength = 1;
			for (GLuint x = 0; x < 8; ++x)
				if (score.m_score >= decimalPlaces[x]) {
					scoreLength = 8-x;
					break;
				}
			const GLuint doubledLength = scoreLength * 2u;
			m_bufferIndirectScore.write(sizeof(GLuint), sizeof(GLuint), &doubledLength);
			m_shaderScore->setUniform(4, scoreLength);

			const GLuint scoreCount = (GLuint)score.m_scoredTiles.size() * 2u;
			m_bufferIndirectTilesScored.write(sizeof(GLuint), sizeof(GLuint), &scoreCount);
			m_shaderTileScored->setUniform(4, (GLuint)score.m_scoredTiles.size());
			for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
				const glm::ivec3 data(score.m_scoredTiles[x].first[0].x, score.m_scoredTiles[x].first[0].y, (int)score.m_scoredTiles[x].first.size());
				m_bufferScoredTileMarkers.write(sizeof(glm::ivec4) * x, sizeof(glm::ivec3), &data);
			}
			
			// Render game tiles to the FBO
			glViewport(0, 0, tileSize * 6, tileSize * 12);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDField);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_shaderTiles->bind();
			m_textureTile->bind(0);
			m_textureTilePlayer->bind(1);
			m_shaderTiles->setUniform(0, m_orthoProjField);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_bufferIndirectTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render score tiles to the FBO
			m_shaderTileScored->bind();
			m_textureTileScored->bind(0);
			m_textureScoreNums->bind(1);
			m_shaderTileScored->setUniform(0, m_orthoProjField);
			m_bufferIndirectTilesScored.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_bufferScoredTileMarkers.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render score header bar to the FBO
			glViewport(0, 0, tileSize * 6, tileSize * 1.5);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDBars);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT);
			m_shaderScore->bind();
			m_shaderScore->setUniform(0, m_orthoProjHeader);
			m_bufferIndirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			
			// Render multiplier into header bar
			m_shaderMultiplier->bind();
			m_shaderMultiplier->setUniform(0, m_orthoProjHeader);
			m_bufferIndirectMultiplier.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render time footer bar to the FBO			
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_shaderTimer->bind();
			m_shaderTimer->setUniform(0, m_orthoProjHeader);
			m_textureTimeStop->bind(0);
			m_bufferIndirectStop.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			
			// Render all of the textures onto the board model
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindVertexArray(*m_vaoModels);
			m_bufferIndirectBoard.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_shaderBoard->bind();
			glBindTextureUnit(0, m_boardTexID);
			glBindTextureUnit(1, m_scoreTexID);
			glBindTextureUnit(2, m_timeTexID);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 4, 0);

			// End
			glDisable(GL_BLEND);
		}
	}

	const bool areAssetsReady() const {
		return (
			m_shapeQuad->existsYet() &&
			m_modelBoard->existsYet() &&
			m_shaderTiles->existsYet() &&
			m_shaderBoard->existsYet() &&
			m_shaderScore->existsYet() &&
			m_shaderMultiplier->existsYet() &&
			m_shaderTimer->existsYet() &&
			m_textureTile->existsYet() &&
			m_textureScoreNums->existsYet() &&
			m_textureTimeStop->existsYet()
		);
	}


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_fboIDField = 0, m_boardTexID = 0, m_fboIDBars = 0, m_scoreTexID = 0, m_timeTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProjField, m_orthoProjHeader;
	Shared_Asset_Primitive m_shapeQuad;

	// Board Rendering Resources
	Shared_Asset_Shader m_shaderBoard;
	Shared_Asset_Model m_modelBoard, m_modelField, m_modelHeader, m_modelFooter;
	StaticBuffer m_bufferIndirectBoard;
	const GLuint * m_vaoModels;

	// Tile Rendering Resources
	Shared_Asset_Shader m_shaderTiles, m_shaderTileScored;
	Shared_Asset_Texture m_textureTile, m_textureTilePlayer, m_textureTileScored;
	StaticBuffer m_bufferIndirectTiles, m_bufferIndirectTilesScored;
	DynamicBuffer m_bufferScoredTileMarkers;

	// Score Rendering Resources
	Shared_Asset_Shader m_shaderScore;
	Shared_Asset_Texture m_textureScoreNums;
	StaticBuffer m_bufferIndirectScore;

	// Multiplier Rendering Resources
	Shared_Asset_Shader m_shaderMultiplier;
	StaticBuffer m_bufferIndirectMultiplier;

	// Stop-Timer Rendering Resources
	Shared_Asset_Shader m_shaderTimer;
	Shared_Asset_Texture m_textureTimeStop;
	StaticBuffer m_bufferIndirectStop;

};

#endif // RENDERING_S_H