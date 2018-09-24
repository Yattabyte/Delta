#include "Assets\Asset_Primitive.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


constexpr char* EXT_PRIMITIVE = ".obj";
constexpr char* DIRECTORY_PRIMITIVE = "\\Primitives\\";

Asset_Primitive::~Asset_Primitive()
{
	if (existsYet())
		glDeleteBuffers(1, &m_uboID);
}

Asset_Primitive::Asset_Primitive(const std::string & filename) : Asset(filename) 
{
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glEnableVertexArrayAttrib(m_vaoID, 1);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribBinding(m_vaoID, 1, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 1, 2, GL_FLOAT, GL_FALSE, 12);
	glCreateBuffers(1, &m_uboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_uboID, 0, sizeof(Single_Primitive_Vertex));
}

Shared_Asset_Primitive Asset_Primitive::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Primitive>(filename, threaded);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Primitive>(filename);
		auto & assetRef = *userAsset.get();

		// Submit the work order
		const std::string &relativePath = DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE;	
		assetManager.submitNewWorkOrder(std::move(std::bind(&initialize, &assetRef, engine, relativePath)), threaded);
	}
	return userAsset;
}

void Asset_Primitive::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Asset_Mesh::Create(engine, relativePath, false);

	const size_t vertexCount = m_mesh->m_geometry.vertices.size();
	m_data.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data[x].vertex = m_mesh->m_geometry.vertices[x];
		m_data[x].uv = m_mesh->m_geometry.texCoords[x];
	}

	// Load Buffers
	const size_t arraySize = m_data.size();
	glNamedBufferStorage(m_uboID, arraySize * sizeof(Single_Primitive_Vertex), &m_data[0], 0);

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize(engine);
}

size_t Asset_Primitive::getSize()
{
	return m_data.size();
}
