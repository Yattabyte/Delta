#pragma once
#ifndef PHYSICS_C_H
#define PHYSICS_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Collider.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


/** A component representing a basic player. */
struct Collider_Component : public ECSComponent<Collider_Component> {
	int m_mass = 0;
	float m_restitution = 1.0f;
	float m_friction = 1.0f;
	Shared_Asset_Collider m_collider;
	btDefaultMotionState * m_motionState = nullptr;
	btRigidBody * m_rigidBody = nullptr;
	Transform m_transform;
};
/** A constructor to aid in creation. */
struct Collider_Constructor : ECSComponentConstructor<Collider_Component> {
	Collider_Constructor(Engine * engine) : m_engine(engine) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Collider_Component();
		component->m_collider = Asset_Collider::Create(m_engine, castAny(parameters[0], std::string("")));
		component->m_mass = castAny(parameters[1], 0);
		component->m_restitution = castAny(parameters[2], 0.0f);
		component->m_friction = castAny(parameters[3], 0.0f);
		return { component, component->ID };		
	}
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PHYSICS_C_H