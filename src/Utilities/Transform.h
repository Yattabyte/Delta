#pragma once
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtx/matrix_decompose.hpp"


/** A 3D transformation object.
Takes in position, orientation, and scaling attributes, and calculates a transformation matrix. */
struct Transform {
	// (De)Constructors
	/** Default Destructor. */
	inline ~Transform() = default;
	/** Default Constructor. */
	inline Transform() = default;
	/** Constructs a transformation object with any of the supplied parameters.
	@param position			the desired position
	@param orientation		the desired orientation
	@param scale			the desired scale */
	Transform(const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale) noexcept;
	/** Constructs a transformation object with only orientation.
	@param orientation	the desired orientation	*/
	explicit Transform(const glm::quat& orientation) noexcept;


	// Public Methods
	/** Recalculates the transformation matrix (and inverse) using this transformations current data. */
	void update() noexcept;
	/** Calculate and return an inverse transform.
	@return				an inverse version of this transform. */
	Transform inverse() noexcept;
	/** Retrieve if this transform is equal to another transform.
	@param	other		the other transform to compare against.
	@return				true if this transform equals the other transform, false otherwise. */
	bool operator==(const Transform& other) const noexcept;
	/** Retrieve if this transform is not equal to another transform.
	@param	other		the other transform to compare against.
	@return				true if this transform is not equal the other transform, false otherwise. */
	bool operator!=(const Transform& other) const noexcept;
	/** Concatenate this transform with another transform.
	@param	other		the other transform to apply to this transform. 
	@return				reference to this transform. */
	Transform& operator*=(const Transform& other) noexcept;
	/** Concatenate this transform with another transform, returning its product.
	@param	other		the other transform to apply to this transform.
	@return				a new transform based on this transform. */
	Transform operator*(const Transform& o) const noexcept;


	// Public Attributes
	//// Input Variables
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1, 0, 0, 0);
	glm::vec3 m_scale = glm::vec3(1.0f);
	//// Derived Variables
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	glm::mat4 m_inverseModelMatrix = glm::mat4(1.0f);
};

#endif // TRANSFORMATION_H