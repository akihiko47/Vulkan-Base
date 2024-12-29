#include "transform.h"

using namespace vu;


void Transform::SetPosition(glm::vec3 position) {
	m_position = position;
}


void Transform::SetScale(glm::vec3 scale) {
	m_scale = scale;
}


void Transform::SetRotation(glm::vec3 rotation) {
	m_rotation = rotation;
}


void Transform::RotateX(float radians) {
	m_rotation.x += radians;
}


void Transform::RotateY(float radians) {
	m_rotation.y += radians;
}


void Transform::RotateZ(float radians) {
	m_rotation.z += radians;
}


glm::vec3 Transform::GetPosition() const {
	return m_position;
}


glm::vec3 Transform::GetScale() const {
	return m_scale;
}


glm::vec3 Transform::GetRotation() const {
	return m_rotation;
}


glm::mat4 Transform::GetModelMatrix() const {
	glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
	glm::mat4 R = glm::rotate(glm::mat4(1.0), m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	R = glm::rotate(R, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	R = glm::rotate(R, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 S = glm::scale(glm::mat4(1.0), m_scale);

	return T * R * S;
}


glm::vec3 Transform::GetForward() const {
	glm::vec3 forward;
	forward.x = cos(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x));
	forward.y = sin(glm::radians(m_rotation.x));
	forward.z = sin(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x));
	forward = glm::normalize(forward);
	return forward;
}


glm::vec3 Transform::GetUp() const {
	glm::vec3 up = glm::normalize(glm::cross(GetForward(), GetRight()));
	return up;
}

glm::vec3 Transform::GetRight() const {
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(GetForward(), up));
	return right;
}


void Transform::BindModelMatrix(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const {
	// update global push constants
	vu::PushConstantsLocal pushConstants{};
	pushConstants.model = GetModelMatrix();

	// bind global push constants
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &pushConstants);
}