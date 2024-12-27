#include "transform.h"

using namespace vu;


void Transform::SetPosition(glm::vec3 position) {
	m_position = position;
}


void Transform::SetScale(glm::vec3 scale) {
	m_scale = scale;
}


void Transform::SetRotation(glm::mat4 rotation) {
	m_rotation = rotation;
}


void Transform::RotateX(float radians) {
	m_rotation = glm::rotate(m_rotation, radians, glm::vec3(1.0f, 0.0f, 0.0f));
}


void Transform::RotateY(float radians) {
	m_rotation = glm::rotate(m_rotation, radians, glm::vec3(0.0f, 1.0f, 0.0f));
}


void Transform::RotateZ(float radians) {
	m_rotation = glm::rotate(m_rotation, radians, glm::vec3(0.0f, 0.0f, 1.0f));
}


glm::vec3 Transform::GetPosition() const {
	return m_position;
}


glm::vec3 Transform::GetScale() const {
	return m_scale;
}


glm::mat4 Transform::GetRotation() const {
	return m_rotation;
}


glm::mat4 Transform::GetModelMatrix() const {
	glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
	glm::mat4 R = m_rotation;
	glm::mat4 S = glm::scale(glm::mat4(1.0), m_scale);

	return T * R * S;
}