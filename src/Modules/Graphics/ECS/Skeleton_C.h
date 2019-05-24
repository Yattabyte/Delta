#pragma once
#ifndef SKELETALANIMATION_C_H
#define SKELETALANIMATION_C_H

#include "Modules/World/ecsComponent.h"
#include "Assets/Mesh.h"


/** OpenGL buffer for animated props.
Formatted to adhere to glsl specifications. */
struct Skeleton_Buffer {
#define NUM_MAX_BONES 100
	glm::mat4 bones[NUM_MAX_BONES];
};
/** A skeleton component is used in combination with prop components to allow for skeletal animation. 
@note		is useless by itself. */
struct Skeleton_Component : public ECSComponent<Skeleton_Component> {
	int m_animation = -1;
	bool m_playAnim = true;
	float m_animTime = 0, m_animStart = 0;
	Shared_Mesh m_mesh;
	std::vector<glm::mat4> m_transforms;
	VB_Element<Skeleton_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct Skeleton_Constructor : ECSComponentConstructor<Skeleton_Component> {
	// Public (de)Constructors
	Skeleton_Constructor(Engine * engine, VectorBuffer<Skeleton_Buffer> * elementBuffer)
		: m_engine(engine), m_elementBuffer(elementBuffer) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto directory = castAny(parameters[0], std::string(""));
		auto animation = castAny(parameters[1], 0);
		auto * component = new Skeleton_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_mesh = Shared_Mesh(m_engine, "\\Models\\" + directory);
		component->m_animation = animation;
		for (int x = 0; x < NUM_MAX_BONES; ++x)
			component->m_data->data->bones[x] = glm::mat4(1.0f);
		return { component, component->ID };
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	VectorBuffer<Skeleton_Buffer> * m_elementBuffer = nullptr;
};

#endif // SKELETALANIMATION_C_H