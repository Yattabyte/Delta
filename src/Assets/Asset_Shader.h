/*
	Asset_Shader

	- Encapsulates an OpenGL shader program
	- Shader program uses multiple shader files with same name
		- Example: "World_Shader.vertex", "World_Shader.fragment", "World_Shader.geometry"
	- Shader uniform values set by binding the appropriate shader, and then modifying the correct location by using the static "setLocation" functions
*/

#pragma once
#ifndef	ASSET_SHADER
#define	ASSET_SHADER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_SHADER_VERTEX ".vsh"
#define EXT_SHADER_FRAGMENT ".fsh"
#define EXT_SHADER_GEOMETRY ".gsh"
#define DIRECTORY_SHADER File_Reader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include <string>

using namespace glm;
using namespace std;

class Asset_Shader;
typedef shared_ptr<Asset_Shader> Shared_Asset_Shader;
class DT_ENGINE_API Asset_Shader : public Asset
{
public:	
	/*************
	----Common----
	*************/

	~Asset_Shader();
	Asset_Shader(const string & filename);
	static int GetAssetType();
	bool ExistsYet();


	/****************
	----Variables----
	****************/

	GLuint gl_program_ID, gl_shader_vertex_ID, gl_shader_fragment_ID, gl_shader_geometry_ID; // OpenGL ID's
	string vertex_text, fragment_text, geometry_text; // Text Data
	GLsync m_fence;


	/***********************
	----Shader Functions----
	***********************/

	// Make this shader program active
	void Bind();
	// Inactivate any currently bound shader program
	static void Release();	


	/****************************************************************************************************
 	----Convenience functions for setting uniform values at a given location, while a shader is bound----
	****************************************************************************************************/

	static void setLocationValue(const GLuint & i, const bool & o);
	static void setLocationValue(const GLuint & i, const int & o);
	static void setLocationValue(const GLuint & i, const double & o);
	static void setLocationValue(const GLuint & i, const float & o);
	static void setLocationValue(const GLuint & i, const vec2 & o);
	static void setLocationValue(const GLuint & i, const vec3 & o);
	static void setLocationValue(const GLuint & i, const vec4 & o);
	static void setLocationValue(const GLuint & i, const ivec2 & o);
	static void setLocationValue(const GLuint & i, const ivec3 & o);
	static void setLocationValue(const GLuint & i, const ivec4 & o);
	static void setLocationValue(const GLuint & i, const mat3 & o);
	static void setLocationValue(const GLuint & i, const mat4 & o);
	static void setLocationValue(const GLuint & i, const int * o);
	static void setLocationValue(const GLuint & i, const double * o);
	static void setLocationValue(const GLuint & i, const float * o);
	static void setLocationValue(const GLuint & i, const vec2 * o);
	static void setLocationValue(const GLuint & i, const vec3 * o);
	static void setLocationValue(const GLuint & i, const vec4 * o);
	static void setLocationValue(const GLuint & i, const mat3 * o);
	static void setLocationValue(const GLuint & i, const mat4 * o);
	static void setLocationValueArray(const GLuint & i, const int & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const double & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const float & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec2 & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec3 & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec4 & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const mat4 & o, const int & size);
	static void setLocationValueArray(const GLuint & i, const int * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const double * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const float * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec2 * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec3 * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const vec4 * o, const int & size);
	static void setLocationValueArray(const GLuint & i, const mat4 * o, const int & size);
	static void setLocationMatArray(const GLuint & i, const float * o, const int & size, const GLboolean & transpose);		
};

namespace Asset_Loader {
	// Attempts to create an asset from disk or share one if it already exists
	DT_ENGINE_API void load_asset(Shared_Asset_Shader & user, const string & filename, const bool & threaded = true);
};

class Shader_WorkOrder : public Work_Order {
public:
	Shader_WorkOrder(Shared_Asset_Shader & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Shader_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	// Parse for inclusions
	void Parse();
	// Creates and compiles all available shader types for this program
	void Compile();
	// Creates and compiles a single shader file, given the text as <source> and the specified shader type as <type>.
	void Compile_Single_Shader(GLuint & ID, const char * source, const GLenum & type);
	// Generates an OpenGL shader program ID for this class, and attempts to attach any available shaders
	void GenerateProgram();
	// Attempts to link and validate the shader program
	void LinkProgram();
	// Reads in a text file from disk, given a file directory, and appends it to the returnFile param
	bool FetchFileFromDisk(string & returnFile, const string & fileDirectory);
	

	/****************
	----Variables----
	****************/

	string m_filename;
	Shared_Asset_Shader m_asset;
};

#endif // ASSET_SHADER