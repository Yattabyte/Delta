/*
	Prop

	- A renderable mesh with a position, orientation, and scale
*/

#pragma once
#ifndef PROP
#define PROP
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "Utilities\Transform.h"
#include "Assets\Asset_Model.h"
#include <string>

using namespace glm;
using namespace std;

struct Transform_Buffer {
	int useBones; vec3 padding1;
	mat4 mMatrix;
	mat4 transforms[NUM_MAX_BONES];
};

class Prop : public Entity
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Prop();
	DELTA_CORE_API Prop();
	DELTA_CORE_API Prop(const string &relativePath);
	DELTA_CORE_API Prop(const Prop &other);
	DELTA_CORE_API void operator= (const Prop &other);


	/*************************
	----Variable Functions----
	*************************/

	void setPosition(const vec3 &position) { worldState.position = position; }
	void setOrientation(const quat &orientation) { worldState.orientation = orientation; }
	void setScale(const vec3 &scale) { worldState.scale = scale; }
	vec3 getPosition() const { return worldState.position; }
	quat getOrientation() const { return worldState.orientation; }
	vec3 getScale() const { return worldState.scale; }
	mat4 getModelMatrix() const { return worldState.modelMatrix; }
	mat4 getInverseModelMatrix() const { return worldState.inverseModelMatrix; }
	DELTA_CORE_API void Update();


	/**************************
	----Rendering Functions----
	**************************/

	DELTA_CORE_API void geometryPass();


	/****************
	----Variables----
	****************/

	Transform worldState;
	Shared_Asset_Model assetModel;
	GLuint uboID;
	Transform_Buffer uboData;
};

#endif // PROP