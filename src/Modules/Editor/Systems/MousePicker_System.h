#pragma once
#ifndef MOUSEPICKER_SYSTEM_H
#define MOUSEPICKER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


/** An ECS system allowing the user to ray-pick entities by selecting against their components. */
class MousePicker_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~MousePicker_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline MousePicker_System(Engine* engine)
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::m_ID);
		addComponentType(BoundingBox_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Collider_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Prop_Component::m_ID, FLAG_OPTIONAL);

		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
			m_renderSize.x = (int)f;
			});
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
			m_renderSize.y = (int)f;
			});
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<ecsBaseComponent*> >& components) override final {
		const auto& actionState = m_engine->getActionState();
		const auto& clientCamera = *m_engine->getModule_Graphics().getClientCamera()->get();
		const auto ray_origin = clientCamera.EyePosition;
		const auto ray_nds = glm::vec2(2.0f * actionState.at(ActionState::MOUSE_X) / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState.at(ActionState::MOUSE_Y)) / m_renderSize.y);
		const auto ray_eye = glm::vec4(glm::vec2(clientCamera.pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
		const auto ray_direction = glm::normalize(glm::vec3(clientCamera.vMatrixInverse * ray_eye));

		// Set the selection position for the worst-case scenario
		m_selectionTransform.m_position = clientCamera.EyePosition + (ray_direction * glm::vec3(50.0f));
		m_intersectionTransform.m_position = clientCamera.EyePosition + (ray_direction * glm::vec3(50.0f));

		float closestDistance = FLT_MAX;
		int highestConfidence = 0;
		bool found = false;
		for each (const auto & componentParam in components) {
			auto* transformComponent = (Transform_Component*)componentParam[0];
			auto* bBox = (BoundingBox_Component*)componentParam[1];
			auto* bSphere = (BoundingSphere_Component*)componentParam[2];
			auto* collider = (Collider_Component*)componentParam[3];
			auto* prop = (Prop_Component*)componentParam[4];
			float distanceFromScreen = FLT_MAX;
			int confidence = 0;
			const bool hasBoundingShape = bSphere || bBox;
			bool result = false;

			// Attempt cheap tests first
			result = RayOrigin(transformComponent, ray_origin, ray_direction, distanceFromScreen, confidence);
			if (bBox)
				result = RayBBox(transformComponent, bBox, ray_origin, ray_direction, distanceFromScreen, confidence);
			else if (bSphere)
				result = RayBSphere(transformComponent, bSphere, ray_origin, ray_direction, distanceFromScreen, confidence);

			// Attempt more complex tests
			if (prop && ((hasBoundingShape && result) || (!hasBoundingShape))) {
				distanceFromScreen = FLT_MAX;
				result = RayProp(transformComponent, prop, ray_origin, ray_direction, distanceFromScreen, confidence);
			}
			else if (collider && ((hasBoundingShape && result) || (!hasBoundingShape))) {
				distanceFromScreen = FLT_MAX;
				result = RayCollider(transformComponent, collider, ray_origin, ray_direction, distanceFromScreen, confidence);
			}

			// Find the closest best match
			if (result && ((distanceFromScreen < closestDistance) || (confidence > highestConfidence))) {
				closestDistance = distanceFromScreen;
				highestConfidence = confidence;
				m_selection = transformComponent->m_entity;
				m_selectionTransform = transformComponent->m_worldTransform;
				found = true;
			}
		}
		if (found) {
			m_intersectionTransform.m_position = ray_origin + closestDistance * ray_direction;
			m_intersectionTransform.update();
		}
	}


	// Public Methods
	/** Retrieve this system's last selection result. */
	std::tuple<EntityHandle, Transform, Transform> getSelection() {
		return { m_selection, m_selectionTransform, m_intersectionTransform };
	}


private:
	// Private Methods
	/** Perform a ray-prop intersection test.
	@param	transformComponent		the transform component of interest.
	@param	prop					the prop component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	bool RayProp(Transform_Component* transformComponent, Prop_Component* prop, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) {
		bool intersection = false;
		if (prop->m_model && prop->m_model->existsYet()) {
			float distance = FLT_MAX;
			for (size_t x = 0; x < prop->m_model->m_data.m_vertices.size(); x += 3) {
				auto v0 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x].vertex, 1);
				auto v1 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x + 1].vertex, 1);
				auto v2 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x + 2].vertex, 1);
				v0 /= v0.w;
				v1 /= v1.w;
				v2 /= v2.w;
				if (RayTriangleIntersection(
					ray_origin, ray_direction,
					glm::vec3(v0), glm::vec3(v1), glm::vec3(v2), glm::vec2(),
					distance
				)) {
					distanceFromScreen = distance;
					confidence = 3;
					intersection = true;
				}
			}
		}
		return intersection;
	}
	/** Perform a ray-collider intersection test.
	@param	transformComponent		the transform component of interest.
	@param	collider				the collider component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	bool RayCollider(Transform_Component* transformComponent, Collider_Component* collider, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) {
		//confidence = 3;
		return false;
	}
	/** Perform a ray-bounding-box intersection test.
	@param	transformComponent		the transform component of interest.
	@param	bBox					the bounding box component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	bool RayBBox(Transform_Component* transformComponent, BoundingBox_Component* bBox, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) {
		float distance;
		const auto& position = transformComponent->m_worldTransform.m_position;
		const auto& scale = transformComponent->m_worldTransform.m_scale;
		Transform newTransform = transformComponent->m_worldTransform;
		newTransform.m_position += bBox->m_positionOffset;
		newTransform.m_scale = glm::vec3(1.0f);
		newTransform.update();
		const auto matrixWithoutScale = newTransform.m_modelMatrix;

		// Check if the distance is closer than the last entity found, so we can find the 'best' selection
		if (RayOOBBIntersection(ray_origin, ray_direction, bBox->m_min * scale, bBox->m_max * scale, matrixWithoutScale, distance)) {
			distanceFromScreen = distance;
			confidence = 2;
			return true;
		}
		return false;
	}
	/** Perform a ray-bounding-sphere intersection test.
	@param	transformComponent		the transform component of interest.
	@param	bSphere					the bounding sphere component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	bool RayBSphere(Transform_Component* transformComponent, BoundingSphere_Component* bSphere, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) {
		// Check if the distance is closer than the last entity found, so we can find the 'best' selection
		if (auto distance = RaySphereIntersection(ray_origin, ray_direction, transformComponent->m_worldTransform.m_position + bSphere->m_positionOffset, bSphere->m_radius); distance >= 0.0f) {
			distanceFromScreen = distance;
			confidence = 2;
			return true;
		}
		return false;
	}
	/** Perform a ray-origin intersection test.
	@param	transformComponent		the transform component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	bool RayOrigin(Transform_Component* transformComponent, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) {
		// Create scaling factor to keep all origins same screen size
		const auto radius = glm::distance(transformComponent->m_worldTransform.m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f;

		// Check if the distance is closer than the last entity found, so we can find the 'best' selection
		if (auto distance = RaySphereIntersection(ray_origin, ray_direction, transformComponent->m_worldTransform.m_position, radius); distance >= 0.0f) {
			distanceFromScreen = distance;
			confidence = 2;
			return true;
		}
		return false;
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	EntityHandle m_selection;
	Transform m_selectionTransform, m_intersectionTransform;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MOUSEPICKER_SYSTEM_H