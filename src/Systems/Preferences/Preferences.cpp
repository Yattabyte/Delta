#include "Systems\Preferences\Preferences.h"
#include "Utilities\EnginePackage.h"


System_Preferences::~System_Preferences()
{
	m_enginePackage->m_PreferenceState.Save();
}

System_Preferences::System_Preferences(const std::string & filename) : m_fileName(filename)
{	
}

void System_Preferences::initialize(EnginePackage * enginePackage)
{ 
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_enginePackage->m_PreferenceState.LoadFile(m_fileName);
		m_enginePackage->m_Camera.setDimensions(vec2(m_enginePackage->getPreference(PreferenceState::C_WINDOW_WIDTH), m_enginePackage->getPreference(PreferenceState::C_WINDOW_HEIGHT)));
		m_enginePackage->m_Camera.setFarPlane(m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE));
		m_enginePackage->m_Camera.setGamma(m_enginePackage->getPreference(PreferenceState::C_GAMMA));
		m_enginePackage->m_Camera.update();
		m_Initialized = true;
	}
}