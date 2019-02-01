#include "Assets/Image.h"
#include "Utilities/IO/Image_IO.h"
#include "Engine.h"
#include "glm/gtc/type_ptr.hpp"


Shared_Image::Shared_Image(Engine * engine, const std::string & filename, const std::optional<glm::ivec2>& specificSize, const bool & threaded, const GLenum & policyFill, const GLenum & policyResize)
	: std::shared_ptr<Image>(std::dynamic_pointer_cast<Image>(engine->getManager_Assets().shareAsset(typeid(Image).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Image>*)(this)) = std::make_shared<Image>(filename, specificSize, policyFill, policyResize);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Image).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Image::initialize, get(), engine, (filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Image::~Image()
{
	delete m_pixelData;
}

Image::Image(const std::string & filename, const std::optional<glm::ivec2> & specificSize, const GLenum & policyFill, const GLenum & policyResize) : Asset(filename), m_policyFill(policyFill), m_policyResize(policyResize) 
{
	if (specificSize)
		m_size = specificSize.value();
}

void Image::initialize(Engine * engine, const std::string & relativePath)
{
	Image_Data dataContainer{ m_pixelData, m_size, m_pitch, m_bpp };
	if (Image_IO::Import_Image(engine, relativePath, dataContainer, m_policyResize)) {
		m_size = dataContainer.dimensions;
		m_pixelData = dataContainer.pixelData;
		m_pitch = dataContainer.pitch;
		m_bpp = dataContainer.bpp;
	}
	else {
		engine->getManager_Messages().error("Image \"" + m_filename + "\" failed to initialize.");
		fill();
	}
	
	Asset::finalize(engine);
}

void Image::fill(const glm::uvec4 primaryColor, const glm::uvec4 secondaryColor)
{
	if (m_size == glm::ivec2(0))
		m_size = glm::ivec2(256);
	const size_t pixelCount = m_size.x * m_size.x;
	const size_t componentCount = pixelCount * 4;
	m_pixelData = new GLubyte[componentCount];
	m_pitch = m_size.x * 4;
	m_bpp = 32;
	switch (m_policyFill) {
		case Solid: {
			size_t pxComponent = 0;
			std::generate(m_pixelData, m_pixelData + componentCount, [&](void)->GLubyte {
				const unsigned int ID = (pxComponent++) % 4;
				const GLubyte component = GLubyte(primaryColor[ID]);
				return component;
			});
			break;
		}
		case Checkered: {
			// How many pixels wide and tall the checkers should be
			constexpr size_t checkerSize = 32;
			const size_t rowWidth = m_size.x;
			const GLubyte colors[2][4] = {
				{ GLubyte(primaryColor.x), GLubyte(primaryColor.y), GLubyte(primaryColor.z), GLubyte(primaryColor.w) },
				{ GLubyte(secondaryColor.x), GLubyte(secondaryColor.y), GLubyte(secondaryColor.z), GLubyte(secondaryColor.w) }
			};
			bool cFlip = false;
			for (size_t pixel = 0, depth = size_t(-1); pixel < pixelCount; ++pixel) {
				// Flip color if we've drawn enough of the checker
				if ((pixel % checkerSize) == 0)
					cFlip = !cFlip;
				// Increment depth when we finish a row
				if ((pixel % rowWidth) == 0) {
					depth++;
					if (depth % checkerSize == 0)
						cFlip = !cFlip;
				}
				// Copy color info into pixel spot
				std::memcpy(&m_pixelData[pixel * 4], colors[cFlip], sizeof(GLubyte) * 4);
			}
			break;
		}
	};
}

void Image::resize(const glm::ivec2 newSize)
{
	Image_Data dataContainer{ m_pixelData, m_size, m_pitch, m_bpp };
	Image_IO::Resize_Image(newSize, dataContainer, m_policyResize);
	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	m_pitch = dataContainer.pitch;
	m_bpp = dataContainer.bpp;
}