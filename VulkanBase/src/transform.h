#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "vu.h"

namespace vu {
	class Transform {
	public:
		Transform() : 
			m_position(0.0f), 
			m_scale(1.0f), 
			m_rotation(0.0f) {};

		Transform(glm::vec3 position) :
			m_position(position),
			m_scale(1.0f),
			m_rotation(0.0f) {};

		Transform(glm::vec3 position, glm::mat4 rotation, glm::vec3 scale) : 
			m_position(position), 
			m_scale(scale), 
			m_rotation(0.0f) {};

		void SetPosition(glm::vec3 position);
		void SetScale(glm::vec3 scale);
		void SetRotation(glm::vec3 rotation);

		void RotateX(float radians);
		void RotateY(float radians);
		void RotateZ(float radians);

		glm::vec3 GetPosition()    const;
		glm::vec3 GetScale()       const;
		glm::vec3 GetRotation()    const;
		glm::mat4 GetModelMatrix() const;
		glm::vec3 GetForward()     const;
		glm::vec3 GetUp()          const;
		glm::vec3 GetRight()       const;

		void BindModelMatrix(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	private:
		glm::vec3 m_position;
		glm::vec3 m_scale;
		glm::vec3 m_rotation;
	};
}

