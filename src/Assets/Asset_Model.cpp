#include "Assets\Asset_Model.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"
#include <minmax.h>


/** Calculates a Axis Aligned Bounding Box from a set of vertices.\n
* Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
* @param	vertices	the vertices of the mesh to derive the AABB from
* @param	minOut	output reference containing the minimum extents of the AABB
* @param	maxOut	output reference containing the maximum extents of the AABB */
inline void calculate_AABB(const vector<vec3> & vertices, vec3 & minOut, vec3 & maxOut, vec3 & centerOut, float & radiusOut)
{
	if (vertices.size() >= 1) {
		const vec3 &vector = vertices.at(0);
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (int x = 1, total = vertices.size(); x < total; ++x) {
			const vec3 &vertex = vertices.at(x);
			if (vertex.x < minX)
				minX = vertex.x;
			else if (vertex.x > maxX)
				maxX = vertex.x;
			if (vertex.y < minY)
				minY = vertex.y;
			else if (vertex.y > maxY)
				maxY = vertex.y;
			if (vertex.z < minZ)
				minZ = vertex.z;
			else if (vertex.z > maxZ)
				maxZ = vertex.z;
		}

		minOut = vec3(minX, minY, minZ);
		maxOut = vec3(maxX, maxY, maxZ);
		centerOut = ((maxOut - minOut) / 2.0f) + minOut;
		radiusOut = glm::distance(minOut, maxOut) / 2.0f;
	}
}

/** Initialize a model's material, where each texture is specified individually.
* @param	engine			the engine being used
* @param	modelMaterial	the material asset to load into
* @param	sceneMaterial	the scene material to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const aiMaterial * sceneMaterial)
{
	// Get the aiStrings for all the textures for a material
	aiString	albedo, normal, metalness, roughness, height, ao;
	aiReturn	albedo_exists = sceneMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedo),
				normal_exists = sceneMaterial->GetTexture(aiTextureType_NORMALS, 0, &normal),
				metalness_exists = sceneMaterial->GetTexture(aiTextureType_SPECULAR, 0, &metalness),
				roughness_exists = sceneMaterial->GetTexture(aiTextureType_SHININESS, 0, &roughness),
				height_exists = sceneMaterial->GetTexture(aiTextureType_HEIGHT, 0, &height),
				ao_exists = sceneMaterial->GetTexture(aiTextureType_AMBIENT, 0, &ao);

	// Assuming the diffuse element exists, generate some fallback texture elements
	std::string templateTexture, extension = ".png";
	if (albedo_exists == AI_SUCCESS) {
		std::string minusD = albedo.C_Str();
		int exspot = minusD.find_last_of(".");
		extension = minusD.substr(exspot, minusD.length());
		int diffuseStart = minusD.find("diff");
		if (diffuseStart > -1)
			minusD = minusD.substr(0, diffuseStart);
		else
			minusD = minusD.substr(0, exspot) + "_";
		templateTexture = minusD;
	}

	// Importer might not distinguish between height and normal maps
	if (normal_exists != AI_SUCCESS && height_exists == AI_SUCCESS) {
		std::string norm_string(height.C_Str());
		const int norm_spot = norm_string.find_last_of("norm");
		if (norm_spot > -1) {
			// Normal map confirmed to be in height map spot, move it over
			normal = height;
			normal_exists = AI_SUCCESS;
			height_exists = AI_FAILURE;
		}
	}

	// Get texture names
	std::string material_textures[6] = {
		/*ALBEDO*/						DIRECTORY_MODEL_MAT_TEX + (albedo_exists == AI_SUCCESS ? albedo.C_Str() : "albedo.png"),
		/*NORMAL*/						DIRECTORY_MODEL_MAT_TEX + (normal_exists == AI_SUCCESS ? normal.C_Str() : templateTexture + "normal" + extension),
		/*METALNESS*/					DIRECTORY_MODEL_MAT_TEX + (metalness_exists == AI_SUCCESS ? metalness.C_Str() : templateTexture + "metalness" + extension),
		/*ROUGHNESS*/					DIRECTORY_MODEL_MAT_TEX + (roughness_exists == AI_SUCCESS ? roughness.C_Str() : templateTexture + "roughness" + extension),
		/*HEIGHT*/						DIRECTORY_MODEL_MAT_TEX + (height_exists == AI_SUCCESS ? height.C_Str() : templateTexture + "height" + extension),
		/*AO*/							DIRECTORY_MODEL_MAT_TEX + (ao_exists == AI_SUCCESS ? ao.C_Str() : templateTexture + "ao" + extension)
	};

	engine->createAsset(modelMaterial, string(""), true, material_textures);
}

/** Initialize a model's materials, using the model's name as a lookup to an external material file.
* @param	engine			the engine being used
* @param	modelMaterial	the material asset to load into
* @param	filename		the model's filename to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const string & filename)
{
	std::string materialFilename = filename.substr(filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	engine->createAsset(modelMaterial, materialFilename, true);
}

Asset_Model::~Asset_Model()
{
	if (existsYet())
		m_modelManager->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const string & filename, ModelManager * modelManager) : Asset(filename)
{
	m_meshSize = 0;
	m_bboxMin = vec3(0.0f);
	m_bboxMax = vec3(0.0f);
	m_bboxCenter = vec3(0.0f);
	m_radius = 0.0f;
	m_offset = 0;
	m_count = 0;
	m_modelManager = modelManager;
}

void Asset_Model::CreateDefault(Engine * engine, Shared_Asset_Model & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultModel"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultModel", &modelManager);
	userAsset->m_data.vs = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.uv = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	userAsset->m_data.nm = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.tg = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.bt = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_meshSize = 6; // Final vertex size (needed for draw arrays call)
	userAsset->m_data.bones.resize(6);
	userAsset->m_skins.resize(1);
	calculate_AABB(userAsset->m_data.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);
	engine->createAsset(userAsset->m_skins[0], string("defaultMaterial"), true);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Model::Create(Engine * engine, Shared_Asset_Model & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_MODEL + filename;
	if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		/* Constructor Arguments. */
		filename, &modelManager
	);
}

void Asset_Model::Initialize(Engine * engine, Shared_Asset_Model & userAsset, const string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_model, dataContainer)) {
		engine->reportError(MessageManager::OTHER_ERROR, "Failed to load model asset, using default...");
		CreateDefault(engine, userAsset);
		return;
	}

	unique_lock<shared_mutex> m_asset_guard(userAsset->m_mutex);
	userAsset->m_meshSize = dataContainer.vertices.size();
	userAsset->m_data.vs = dataContainer.vertices;
	userAsset->m_data.nm = dataContainer.normals;
	userAsset->m_data.tg = dataContainer.tangents;
	userAsset->m_data.bt = dataContainer.bitangents;
	userAsset->m_data.uv = dataContainer.texCoords;
	userAsset->m_data.bones = dataContainer.bones;
	userAsset->m_boneTransforms = dataContainer.boneTransforms;
	userAsset->m_boneMap = dataContainer.boneMap;
	userAsset->m_animations = dataContainer.animations;
	userAsset->m_rootNode = dataContainer.rootNode;

	calculate_AABB(userAsset->m_data.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);

	// Generate all the required skins
	userAsset->m_skins.resize(max(1, (dataContainer.materials.size() - 1)));
	if (dataContainer.materials.size() > 1)
		for (int x = 1; x < dataContainer.materials.size(); ++x) // ignore scene material [0] 
			generate_material(engine, userAsset->m_skins[x - 1], dataContainer.materials[x]);
	else
		generate_material(engine, userAsset->m_skins[0], fullDirectory);
}

void Asset_Model::Finalize(Engine * engine, Shared_Asset_Model & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Register geometry
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	userAsset->m_modelManager->registerGeometry(userAsset->m_data, userAsset->m_offset, userAsset->m_count);

	// Notify Completion
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
}

GLuint Asset_Model::getSkinID(const unsigned int & desired)
{
	shared_lock<shared_mutex> guard(m_mutex);
	return m_skins[max(0, min(m_skins.size() - 1, desired))]->m_matSpot;
}