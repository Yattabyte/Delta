#pragma once
#ifndef ECSENTITY_H
#define ECSENTITY_H

#include <string>
#include <vector>


/** An entity in the ECS paradigm - holds components and heirarchical information. */
struct ecsEntity {
	// A non-unique name used primarily for level editing and debugging
	std::string m_name = "Entity";
	// A universally unique identifier
	std::string m_uuid = std::string(32,'0');
	// The index this entity is found at in the entity vector
	int m_entityIndex = -1;
	// The list of component raw-types held by this entity. First of pair is the component ID, second is id to the create function
	std::vector<std::pair<int, int>> m_components = {};
	// An optional parent for this entity, used when forming larger mega-entities
	ecsEntity* m_parent = nullptr;
	// An optional set of children for this entity, whom this entity will be the parent of
	std::vector<ecsEntity*> m_children;
};

#endif // ECSENTITY_H