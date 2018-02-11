/*
	Prop

	- A type of entity
	- Contains an animation-supported model-component
*/

#pragma once
#ifndef PROP
#define PROP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

class DT_ENGINE_API PropCreator : public EntityCreator
{
public:
	virtual Entity* Create(const ECShandle &id, ECSmessanger *ecsMessanger, Component_Factory *componentFactory) {
		Entity *entity = EntityCreator::Create(id, ecsMessanger, componentFactory);
		entity->addComponent("Anim_Model");
		return entity;
	}
};

#endif // PROP