#include "Assets\Asset_Primitive.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"


Asset_Primitive::~Asset_Primitive()
{
	if (existsYet())
		glDeleteBuffers(2, m_buffers);
}

Asset_Primitive::Asset_Primitive(const string & filename) : Asset(filename)
{
	for each (auto &buffer in m_buffers)
		buffer = -1;
}

void Asset_Primitive::CreateDefault(Engine * engine, Shared_Asset_Primitive & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultPrimitive"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultPrimitive");
	userAsset->m_dataVertex = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_dataUV = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Primitive::Create(Engine * engine, Shared_Asset_Primitive & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = ABS_DIRECTORY_PRIMITIVE(filename);
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
		filename
	);
}

void Asset_Primitive::Initialize(Engine * engine, Shared_Asset_Primitive & userAsset, const string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_primitive, dataContainer)) {
		engine->reportError(MessageManager::OTHER_ERROR, "Failed to load primitive asset, using default asset...");
		CreateDefault(engine, userAsset);
		return;
	}


	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_dataVertex = dataContainer.vertices;
	userAsset->m_dataUV = dataContainer.texCoords;
}

void Asset_Primitive::Finalize(Engine * engine, Shared_Asset_Primitive & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();	
	userAsset->finalize();

	// Create buffers
	{
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		glCreateBuffers(2, userAsset->m_buffers);
	}

	// Load Buffers
	{
		shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
		glNamedBufferStorage(userAsset->m_buffers[0], userAsset->m_dataVertex.size() * sizeof(vec3), &userAsset->m_dataVertex[0][0], GL_CLIENT_STORAGE_BIT);
		glNamedBufferStorage(userAsset->m_buffers[1], userAsset->m_dataVertex.size() * sizeof(vec2), &userAsset->m_dataUV[0][0], GL_CLIENT_STORAGE_BIT);
		
		// Notify Completion
		for each (auto qwe in userAsset->m_callbacks)
			assetManager.submitNotifyee(qwe.second);
	}
}

GLuint Asset_Primitive::Generate_VAO()
{
	GLuint vaoID = 0;

	glCreateVertexArrays(1, &vaoID);
	glEnableVertexArrayAttrib(vaoID, 0);
	glEnableVertexArrayAttrib(vaoID, 1);

	return vaoID;
}

void Asset_Primitive::updateVAO(const GLuint & vaoID)
{
	shared_lock<shared_mutex> guard(m_mutex);
	
	glVertexArrayAttribFormat(vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vaoID, 1, 2, GL_FLOAT, GL_FALSE, 0);

	glVertexArrayVertexBuffer(vaoID, 0, m_buffers[0], 0, 12);
	glVertexArrayVertexBuffer(vaoID, 1, m_buffers[1], 0, 8);

	glVertexArrayAttribBinding(vaoID, 0, 0);
	glVertexArrayAttribBinding(vaoID, 1, 1);
}

size_t Asset_Primitive::getSize()
{
	shared_lock<shared_mutex> guard(m_mutex);
	return m_dataVertex.size();
}
