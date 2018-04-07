#include "Assets\Asset_Primitive.h"
#include "Managers\Message_Manager.h"
#include "Utilities\Model_Importer.h"
#include "ASSIMP\Importer.hpp"
#include "ASSIMP\postprocess.h"
#include "ASSIMP\scene.h"


Asset_Primitive::~Asset_Primitive()
{
	if (existsYet())
		glDeleteBuffers(2, m_buffers);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Primitive::Asset_Primitive(const string & filename) : Asset(filename)
{
	for each (auto &buffer in m_buffers)
		buffer = -1;
	m_fence = nullptr;
}

bool Asset_Primitive::existsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::existsYet() && m_fence != nullptr) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED))
			return true;
	}
	return false;
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

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Primitive & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Primitive>(asset, "defaultPrimitive"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Primitive>(asset, "defaultPrimitive");
	asset->m_dataVertex = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->m_dataUV = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	Asset_Manager::Add_Work_Order(new Primitive_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Primitive & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Primitive>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_PRIMITIVE(filename);
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Primitive, Primitive_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Primitive_WorkOrder::initializeOrder()
{
	vector<vec3> vertices;
	vector<vec2> uv_coords;
	if (!Model_Importer::import_Model(m_filename, aiProcess_LimitBoneWeights | aiProcess_Triangulate, vertices, uv_coords)) {
		fetch_default_asset(m_asset);
		return;
	}

	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	m_asset->m_dataVertex = vertices;
	m_asset->m_dataUV = uv_coords;
}

void Primitive_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);

		auto &data = m_asset->m_dataVertex;
		auto &uv_data = m_asset->m_dataUV;
		auto &buffers = m_asset->m_buffers;
		const size_t &arraySize = data.size();
		constexpr GLbitfield flags = GL_CLIENT_STORAGE_BIT;
		glCreateBuffers(2, buffers);
		glNamedBufferStorage(buffers[0], arraySize * sizeof(vec3), &data[0][0], flags);
		glNamedBufferStorage(buffers[1], arraySize * sizeof(vec2), &uv_data[0][0], flags);
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}
}
