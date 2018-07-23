#include "Systems\World\Camera.h"
#include "glm\mat4x4.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>


Camera::~Camera()
{
}

Camera::Camera(const glm::vec3 & position, const glm::vec2 & size, const float & near_plane, const float & far_plane, const float & horizontal_FOV)
{
	setPosition(position);
	setDimensions(size);
	setNearPlane(near_plane);
	setFarPlane(far_plane);
	setHorizontalFOV(horizontal_FOV);
	setOrientation(glm::quat(1, 0, 0, 0));
	enableRendering(true);
	m_buffer = StaticBuffer(sizeof(Camera_Buffer), &m_cameraBuffer);
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
	update();
}

Camera::Camera(Camera const & other)
{
	std::shared_lock<std::shared_mutex> rguard(other.data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	m_buffer = StaticBuffer(sizeof(Camera_Buffer), &m_cameraBuffer);
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
	update();
}

void Camera::operator=(Camera const & other)
{
	std::shared_lock<std::shared_mutex> rguard(other.data_mutex);
	std::unique_lock<std::shared_mutex> wguard(data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	data_mutex.unlock();
	update();
}

void Camera::setMatrices(const glm::mat4 & pMatrix, const glm::mat4 & vMatrix)
{
	std::unique_lock<std::shared_mutex> wguard(data_mutex);
	m_cameraBuffer.pMatrix = pMatrix;
	m_cameraBuffer.vMatrix = vMatrix;
	m_cameraBuffer.pMatrix_Inverse = glm::inverse(pMatrix);
	m_cameraBuffer.vMatrix_Inverse = glm::inverse(vMatrix);

	// Send data to GPU
	m_buffer.write_immediate(0, sizeof(Camera_Buffer), &m_cameraBuffer);
}

void Camera::setVisibilityToken(const Visibility_Token & vis_token)
{
	std::unique_lock<std::shared_mutex> wguard(data_mutex);
	m_vistoken = vis_token;
}

void Camera::update()
{
	std::unique_lock<std::shared_mutex> wguard(data_mutex);

	// Update Perspective Matrix
	float ar = max(1.0f, m_cameraBuffer.Dimensions.x) / max(1.0f, m_cameraBuffer.Dimensions.y);
	float horizontalRad = glm::radians(m_cameraBuffer.FOV);
	float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	m_cameraBuffer.pMatrix = glm::perspective(verticalRad, ar, m_cameraBuffer.NearPlane, m_cameraBuffer.FarPlane);
	m_cameraBuffer.pMatrix_Inverse = glm::inverse(m_cameraBuffer.pMatrix);

	// Update Viewing Matrix
	m_cameraBuffer.vMatrix = glm::mat4_cast(m_orientation) * translate(glm::mat4(1.0f), -m_cameraBuffer.EyePosition);
	m_cameraBuffer.vMatrix_Inverse = glm::inverse(m_cameraBuffer.vMatrix);
	
	// Send data to GPU
	m_buffer.write_immediate(0, sizeof(Camera_Buffer), &m_cameraBuffer);
}

void Camera::bind() const
{
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
}
