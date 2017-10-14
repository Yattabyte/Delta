#include "Plugins\dt_FreeImage.h"
#include "Managers\Material_Manager.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"

using namespace Asset_Manager;

namespace dt_FreeImage {
	FIBITMAP * FetchImageFromDisk(const std::string &fileName, vec2 & dimensions, int & dataSize, bool & success)
	{
		success = true;
		const string &str = fileName;
		const char * file = str.c_str();
		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);
		GLubyte* textureData = nullptr;

		if (format == -1) {
			MSG::Error(FILE_MISSING, str);
			success = false;
		}
		else if (format == FIF_UNKNOWN) {
			MSG::Error(FILE_CORRUPT, str);
			format = FreeImage_GetFIFFromFilename(file);

			if (!FreeImage_FIFSupportsReading(format)) {
				MSG::Statement("Failure, could not recover the file!");
				success = false;
			}
			else {
				MSG::Statement("Successfully resolved the texture file's format!");
				success = true;
			}
		}
		else if (format == FIF_GIF) {
			MSG::Statement("GIF loading unsupported!");
			success = false;
		}

		if (success)
			return FreeImage_Load(format, file);
		return nullptr;
	}

	GLubyte * ReadImage_1channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
	{
		FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
		if (success) {
			FIBITMAP *bitmap32;
			const int bitsPerPixel = FreeImage_GetBPP(bitmap);
			if (bitsPerPixel == 32)
				bitmap32 = bitmap;
			else
				bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

			dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
			dataSize = 1 * (int)dimensions.x * (int)dimensions.y;
			GLubyte* textureData = new GLubyte[dataSize];
			char* pixels = (char*)FreeImage_GetBits(bitmap32);

			for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
				// format gets read as BGRA
				GLubyte blue = pixels[j * 4 + 0],
					green = pixels[j * 4 + 1],
					red = pixels[j * 4 + 2],
					alpha = pixels[j * 4 + 3];

				// store as RGBA
				textureData[j * 1 + 0] = red;
			}

			//Unload
			FreeImage_Unload(bitmap32);
			if (bitsPerPixel != 32)
				FreeImage_Unload(bitmap);
			return textureData;
		}
		return nullptr;
	}

	GLubyte * ReadImage_2channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
	{
		FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
		if (success) {
			FIBITMAP *bitmap32;
			const int bitsPerPixel = FreeImage_GetBPP(bitmap);
			if (bitsPerPixel == 32)
				bitmap32 = bitmap;
			else
				bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

			dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
			dataSize = 2 * (int)dimensions.x * (int)dimensions.y;
			GLubyte* textureData = new GLubyte[dataSize];
			char* pixels = (char*)FreeImage_GetBits(bitmap32);

			for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
				// format gets read as BGRA
				GLubyte blue = pixels[j * 4 + 0],
					green = pixels[j * 4 + 1],
					red = pixels[j * 4 + 2],
					alpha = pixels[j * 4 + 3];

				// store as RGBA
				textureData[j * 2 + 0] = red;
				textureData[j * 2 + 1] = green;
			}

			//Unload
			FreeImage_Unload(bitmap32);
			if (bitsPerPixel != 32)
				FreeImage_Unload(bitmap);
			return textureData;
		}
		return nullptr;
	}

	GLubyte * ReadImage_3channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
	{
		FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
		if (success) {
			FIBITMAP *bitmap32;
			const int bitsPerPixel = FreeImage_GetBPP(bitmap);
			if (bitsPerPixel == 32)
				bitmap32 = bitmap;
			else
				bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

			dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
			dataSize = 3 * (int)dimensions.x * (int)dimensions.y;
			GLubyte* textureData = new GLubyte[dataSize];
			char* pixels = (char*)FreeImage_GetBits(bitmap32);

			for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
				// format gets read as BGRA
				GLubyte blue = pixels[j * 4 + 0],
					green = pixels[j * 4 + 1],
					red = pixels[j * 4 + 2],
					alpha = pixels[j * 4 + 3];

				// store as RGBA
				textureData[j * 3 + 0] = red;
				textureData[j * 3 + 1] = green;
				textureData[j * 3 + 2] = blue;
			}

			//Unload
			FreeImage_Unload(bitmap32);
			if (bitsPerPixel != 32)
				FreeImage_Unload(bitmap);
			return textureData;
		}
		return nullptr;
	}

	GLubyte * ReadImage_4channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
	{
		FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
		if (success) {
			FIBITMAP *bitmap32;
			const int bitsPerPixel = FreeImage_GetBPP(bitmap);
			if (bitsPerPixel == 32)
				bitmap32 = bitmap;
			else
				bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

			dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
			dataSize = 4 * (int)dimensions.x * (int)dimensions.y;
			GLubyte* textureData = new GLubyte[dataSize];
			char* pixels = (char*)FreeImage_GetBits(bitmap32);

			for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
				// format gets read as BGRA
				GLubyte blue = pixels[j * 4 + 0],
					green = pixels[j * 4 + 1],
					red = pixels[j * 4 + 2],
					alpha = pixels[j * 4 + 3];

				// store as RGBA
				textureData[j * 4 + 0] = red;
				textureData[j * 4 + 1] = green;
				textureData[j * 4 + 2] = blue;
				textureData[j * 4 + 3] = alpha;
			}

			//Unload
			FreeImage_Unload(bitmap32);
			if (bitsPerPixel != 32)
				FreeImage_Unload(bitmap);
			return textureData;
		}
		return nullptr;
	}
}

Shared_Asset_Texture fetchDefaultAsset_Texture()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(pair<int, Shared_Asset>(Asset_Texture::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Texture::GetAssetType()];
	if (default_asset.get() == nullptr)
		default_asset = shared_ptr<Asset_Texture>(new Asset_Texture());
	return dynamic_pointer_cast<Asset_Texture>(default_asset);
}

void initialize_Texture(Shared_Asset_Texture &user, const string & filename, bool *complete)
{
	const char * file = filename.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

	if (format == -1) {
		MSG::Error(FILE_MISSING, filename);
		user = fetchDefaultAsset_Texture();
		return;
	}

	if (format == FIF_UNKNOWN) {
		MSG::Error(FILE_CORRUPT, filename);
		format = FreeImage_GetFIFFromFilename(file);
		if (!FreeImage_FIFSupportsReading(format)) {
			//	SceneConsole::Statement("Failure, could not read the file! Using fallback texture...", CERROR);
			user = fetchDefaultAsset_Texture();
			return;
		}
		else
		{//	SceneConsole::Statement("Successfully resolved the texture file's format!", CSUCCESS);
		}
	}

	FIBITMAP* bitmap, *bitmap32;
	FIMULTIBITMAP* mbitmap;

	//Load
	if (format == FIF_GIF) {
		mbitmap = FreeImage_OpenMultiBitmap(FIF_GIF, file, false, true, false, GIF_PLAYBACK);
		MSG::Statement("GIF loading unsupported, using first frame...");
		bitmap = FreeImage_LockPage(mbitmap, 0);
	}
	else
		bitmap = FreeImage_Load(format, file);

	int bitsPerPixel = FreeImage_GetBPP(bitmap);
	if (bitsPerPixel == 32)
		bitmap32 = bitmap;
	else
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

	vec2 size = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
	user->size = size;
	user->pixel_data = new GLubyte[4 * (int)size.x*(int)size.y];
	GLubyte *textureData = user->pixel_data;
	char* pixels = (char*)FreeImage_GetBits(bitmap32);

	for (int j = 0, total = (int)size.x*(int)size.y; j < total; j++) {
		GLubyte blue = pixels[j * 4 + 0],
			green = pixels[j * 4 + 1],
			red = pixels[j * 4 + 2],
			alpha = pixels[j * 4 + 3];
		textureData[j * 4 + 0] = red;
		textureData[j * 4 + 1] = green;
		textureData[j * 4 + 2] = blue;
		textureData[j * 4 + 3] = alpha;
	}

	//Unload
	FreeImage_Unload(bitmap32);
	if (bitsPerPixel != 32)
		FreeImage_Unload(bitmap);

	if (size.x < 1.5f || size.y < 1.5f)
		user->type = GL_TEXTURE_1D;

	submitWorkorder(user);
	*complete = true;
}

void initialize_Material(Shared_Asset_Material &material, const std::string &filename, bool *complete)
{
	if (filename != "") {
		Asset_Material::getPBRProperties(filename, material->textures[0], material->textures[1], material->textures[2], material->textures[3], material->textures[4], material->textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			material->textures[x] = getCurrentDir() + "\\Textures\\" + material->textures[x];
	}

	vec2 material_dimensions = vec2(0);
	{
		unique_lock<shared_mutex> surface_guard(material->m_mutex);

		bool success[MAX_PHYSICAL_IMAGES];
		vec2 dimensions[MAX_PHYSICAL_IMAGES];
		int dataSize[MAX_PHYSICAL_IMAGES];
		GLubyte *textureData[MAX_PHYSICAL_IMAGES];

		textureData[0] = dt_FreeImage::ReadImage_4channel(material->textures[0], dimensions[0], dataSize[0], success[0]);
		textureData[1] = dt_FreeImage::ReadImage_3channel(material->textures[1], dimensions[1], dataSize[1], success[1]);
		for (int x = 2; x < MAX_PHYSICAL_IMAGES; ++x)
			textureData[x] = dt_FreeImage::ReadImage_1channel(material->textures[x], dimensions[x], dataSize[x], success[x]);

		// Material MUST be entirely the same dimensions
		// enforce the first found dimension
		material_dimensions = dimensions[0];
		for (int x = 1; x < MAX_PHYSICAL_IMAGES; ++x) {
			if (success[x]) {
				if (material_dimensions.x < dimensions[x].x)
					material_dimensions.x = dimensions[x].x;
				if (material_dimensions.y < dimensions[x].y)
					material_dimensions.y = dimensions[x].y;
			}
		}

		// If we didn't load a single damn file
		bool did_we_ever_succeed = false;
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			did_we_ever_succeed += success[x];
		if (!did_we_ever_succeed)
			material_dimensions = vec2(1);


		// Stitch data together 
		{	// 3 textures with 4 data channels (RGBA) of X*Y size
			const int mat_data_size_1 = int(material_dimensions.x * material_dimensions.y);
			const int mat_data_size_2 = mat_data_size_1 * 2;
			const int mat_data_size_3 = mat_data_size_1 * 3;
			const int mat_data_size_4 = mat_data_size_1 * 4;
			const int material_data_size = MAX_DIGITAL_IMAGES * mat_data_size_4;
			material->materialData = new GLubyte[material_data_size];
			GLubyte *materialData = material->materialData;

			// Running through whole thing, setting defaults in case an image is smaller than the max size
			// First texture has white albedo with full alpha
			for (int x = 0, size = mat_data_size_4; x < size; ++x)
				materialData[x] = GLubyte(255);
			// Second texture has straight pointing normal with no height
			for (int x = mat_data_size_4, size = mat_data_size_4 * 2; x < size; x += 4) {
				materialData[x + 0] = GLubyte(127);
				materialData[x + 1] = GLubyte(127);
				materialData[x + 2] = GLubyte(255);
				materialData[x + 3] = GLubyte(0);
			}
			// Third texture has quarter metalness (mostly dielectric), half roughness, empty third channel, and full ambience
			for (int x = mat_data_size_4 * 2, size = mat_data_size_4 * 3; x < size; x += 4) {
				materialData[x + 0] = GLubyte(63);
				materialData[x + 1] = GLubyte(127);
				materialData[x + 2] = GLubyte(0);
				materialData[x + 3] = GLubyte(255);
			}

			// ALBEDO
			if (success[0])
				// Fill with the minimum available amount of data, either from the pool or limited to max amount
				// ORDER R, G, B, A, R, G, B, A, etc...
				for (int x = 0, mat_data_spot = 0, total = min(dataSize[0], mat_data_size_4); x < total; ++x, ++mat_data_spot)
					materialData[mat_data_spot] = textureData[0][x];

			// NORMAL
			if (success[1])
				for (int n_x = 0, mat_data_spot = mat_data_size_4, total = min(dataSize[1], mat_data_size_3); n_x < total; n_x += 3, mat_data_spot += 4) {
					materialData[mat_data_spot] = textureData[1][n_x];
					materialData[mat_data_spot + 1] = textureData[1][n_x + 1];
					materialData[mat_data_spot + 2] = textureData[1][n_x + 2];
				}

			// METALNESS
			if (success[2])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, m_t = min(dataSize[2], mat_data_size_1); x < m_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot] = textureData[2][x];

			// ROUGHNESS
			if (success[3])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, r_t = min(dataSize[3], mat_data_size_1); x < r_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot + 1] = textureData[3][x];

			// HEIGHT
			if (success[4])
				for (int h_x = 0, mat_data_spot = mat_data_size_4, total = min(dataSize[4], mat_data_size_3); h_x < total; h_x++, mat_data_spot += 4)
					materialData[mat_data_spot + 3] = textureData[4][h_x];

			// AO
			if (success[5])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, a_t = min(dataSize[5], mat_data_size_1); x < a_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot + 3] = textureData[5][x];
		}

		// Delete old data
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			if (success[x])
				delete textureData[x];
	}
	material->size = material_dimensions;
	submitWorkorder(material);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Texture &user, const string &filename, const bool &mipmap, const bool &anis, const bool &threaded)
	{
		// Check if a copy already finalized
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_textures = (fetchAssetList(Asset_Texture::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_textures) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Texture derived_asset = dynamic_pointer_cast<Asset_Texture>(asset);
				if (derived_asset) {
					if (derived_asset->filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const string &fulldirectory = getCurrentDir() + "\\Textures\\" + filename;
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset_Texture();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Texture(new Asset_Texture(filename, GL_TEXTURE_2D, mipmap, anis));
			assets_textures.push_back(user);
		}

		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Texture, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Texture(user, fulldirectory, complete);
			user->Finalize();
		}
	}
	void load_asset(Shared_Asset_Material &user, const std::string(&textures)[6], const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_materials = (fetchAssetList(Asset_Material::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_materials) {
				bool identical = true;
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Material derived_asset = dynamic_pointer_cast<Asset_Material>(asset);
				if (derived_asset) {
					for (int x = 0; x < 5; ++x) {
						if (derived_asset->textures[x] != textures[x]) {
							identical = false;
							break;
						}
					}
					if (identical) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		{
			// Attempt to create the asset
			unique_lock<shared_mutex> guard(mutex_IO_assets);

			int array_spot = assets_materials.size();
			deque<int> &m_freed_material_spots = Material_Manager::getMatFreeSpots();
			if (m_freed_material_spots.size()) {
				array_spot = m_freed_material_spots.front();
				m_freed_material_spots.pop_front();
			}

			user = Shared_Asset_Material(new Asset_Material(textures, Material_Manager::getBufferSSBO(), array_spot));
			assets_materials.push_back(user);
		}

		// The texture ID of the surface that requested this texture has had it's ID pushed into this material's user list
		// We generate the texture object on the materials handle, and then when processing the work order we propagate the ID onto all the users
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Material, user, "", complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Material(user, "", complete);
			user->Finalize();
		}
	}
	void load_asset(Shared_Asset_Material &user, const std::string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_materials = (fetchAssetList(Asset_Material::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_materials) {
				bool identical = true;
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Material derived_asset = dynamic_pointer_cast<Asset_Material>(asset);
				if (derived_asset) {
					if (derived_asset->material_filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = getCurrentDir() + "\\Materials\\" + filename;
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(DIRECTORY_MISSING, fulldirectory);
			
			// NEED TO UPDATE USER WITH SOMETHING
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);

			int array_spot = assets_materials.size();
			deque<int> &m_freed_material_spots = Material_Manager::getMatFreeSpots();
			if (m_freed_material_spots.size()) {
				array_spot = m_freed_material_spots.front();
				m_freed_material_spots.pop_front();
			}

			user = Shared_Asset_Material(new Asset_Material(filename, Material_Manager::getBufferSSBO(), array_spot));
			assets_materials.push_back(user);
		}

		// The texture ID of the surface that requested this texture has had it's ID pushed into this material's user list
		// We generate the texture object on the materials handle, and then when processing the work order we propagate the ID onto all the users
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Material, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Material(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}