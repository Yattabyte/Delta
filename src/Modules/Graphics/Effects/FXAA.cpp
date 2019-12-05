#include "Modules/Graphics/Effects/FXAA.h"
#include "Engine.h"


FXAA::~FXAA() noexcept 
{
	// Update indicator
	*m_aliveIndicator = false;
}

 FXAA::FXAA(Engine& engine) noexcept :
	Graphics_Technique(Technique_Category::POST_PROCESSING),
	m_engine(engine),
	m_shaderFXAA(Shared_Shader(engine, "Effects\\FXAA")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_FXAA, m_enabled);
	preferences.addCallback(PreferenceState::Preference::C_FXAA, m_aliveIndicator, [&](const float& f) { m_enabled = (bool)f; });
}
 
void FXAA::clearCache(const float& deltaTime) noexcept 
{
	m_drawIndex = 0;
}

void FXAA::renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept 
{
	if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
		return;

	// Prepare camera index
	if (m_drawIndex >= m_drawData.size())
		m_drawData.resize(size_t(m_drawIndex) + 1ull);
	auto& [camBufferIndex, indirectQuad] = m_drawData[m_drawIndex];
	camBufferIndex.beginWriting();
	indirectQuad.beginWriting();
	std::vector<glm::ivec2> camIndices;
	for (auto& [camIndex, layer] : perspectives)
		camIndices.push_back({ camIndex, layer });
	camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
	indirectQuad.setPrimitiveCount((GLuint)perspectives.size());
	camBufferIndex.endWriting();
	indirectQuad.endWriting();

	// Apply FXAA effect
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	viewport->m_gfxFBOS.bindForWriting("FXAA");
	viewport->m_gfxFBOS.bindForReading("HDR", 0);
	m_shaderFXAA->bind();
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.drawCall();

	// Bind for reading by next effect
	glBindTextureUnit(0, viewport->m_gfxFBOS.getTexID("FXAA", 0));
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}