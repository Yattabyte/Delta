#include "Assets\Asset_Config.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include <fstream>

#define EXT_CONFIG ".cfg"
#define DIRECTORY_CONFIG Engine::Get_Current_Dir() + "\\Configs\\"
#define ABS_DIRECTORY_CONFIG(filename) DIRECTORY_CONFIG + filename + EXT_CONFIG


/** Attempts to retrieve a std::string between quotation marks "<std::string>".
@return	the std::string between quotation marks */
inline std::string get_between_quotes(std::string & s)
{
	std::string output = s;
	int spot1 = s.find_first_of("\"");
	if (spot1 >= 0) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		int spot2 = output.find_first_of("\"");
		if (spot2 >= 0) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

/** Checks if the supplied value is a parameter in the CFG_STRING list.
@param	s			the std::string to check for in the list
@param	m_strings	the list of strings to check for an occurrence of our value within
@return				the index of the value in the list if found, otherwise -1 */
inline int find_CFG_Property(const std::string & s, const std::vector<std::string> & m_strings)
{
	std::string UPPER_STRING;
	for each (const auto &c in s)
		UPPER_STRING += toupper(c);
	bool success = false;
	for (auto value = begin(m_strings); value != end(m_strings); value++)
		if ((*value) == UPPER_STRING)
			return std::distance(m_strings.begin(), value);
	return -1;
}

Asset_Config::~Asset_Config()
{
}

Asset_Config::Asset_Config(const std::string & filename, const std::vector<std::string> & strings) : Asset(filename), m_strings(strings)
{
}

Shared_Asset_Config Asset_Config::Create(Engine * engine, const std::string & filename, const std::vector<std::string>& cfg_strings, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Config>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Config>(filename, cfg_strings);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_CONFIG(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Config::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative	
}

void Asset_Config::initialize(Engine * engine, const std::string & fullDirectory)
{
	try {
		std::ifstream file_stream(fullDirectory);
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		for (std::string line; std::getline(file_stream, line); ) {
			if (line.length()) {
				const std::string cfg_property = get_between_quotes(line);
				int spot = find_CFG_Property(cfg_property, m_strings);
				if (spot >= 0) {
					std::string cfg_value = get_between_quotes(line);
					setValue(spot, atof(cfg_value.c_str()));
				}
			}
		}
	}
	catch (const std::ifstream::failure& e) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Config");
		initializeDefault(engine);
		return;
	}
}

void Asset_Config::finalize(Engine * engine)
{
	Asset::finalize(engine);
}

void Asset_Config::setValue(const unsigned int & cfg_key, const float & cfg_value)
{
	// Try inserting the value by key in case the key doesn't exist.
	m_configuration.insert(std::pair<int, float>(cfg_key, cfg_value));
	m_configuration[cfg_key] = cfg_value;
}

float Asset_Config::getValue(const unsigned int & cfg_key)
{
	if (cfg_key >= 0 && m_configuration.find(cfg_key) != m_configuration.end())
		return m_configuration[cfg_key];
	return UNDEFINED_CVAL;
}

void Asset_Config::saveConfig()
{
	std::string output;
	for each (const auto &value in m_configuration) 
		output += "\"" + m_strings[value.first] + "\" \"" + std::to_string(value.second) + "\"\n";
	Text_IO::Export_Text(ABS_DIRECTORY_CONFIG(getFileName()), output);
}