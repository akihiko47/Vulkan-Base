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
	glm::mat4 viewInv = glm::inverse(GetModelMatrix());
	glm::vec3 forward = glm::normalize(glm::vec3(viewInv[2]));
	return forward;
}


glm::vec3 Transform::GetUp() const {
	glm::vec3 up = glm::cross(GetForward(), GetRight());
	return up;
}

glm::vec3 Transform::GetRight() const {
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::normalize(glm::cross(up, GetForward()));
	return right;
}