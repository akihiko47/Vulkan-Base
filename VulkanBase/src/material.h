#pragma once

#include <array>
#include <iostream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

#include "vu.h"

namespace vu {

	class Material {
	public:
		void Initialize(
			VmaAllocator allocator,
			VkSurfaceKHR surface,
			VkFormat swapChainImageFormat,
			const int max_frames_in_flight,
			VkPhysicalDevice physicalDevice,
			VkDevice device,
			VkExtent2D swapChainExtent,
			VkSampleCountFlagBits msaaSamples,
			VkRenderPass renderPass
		);
		void SetResourses(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, VkImageView textureImageView, VkSampler textureSampler);

	private:


		// resources
		VkImageView    m_textureImageView;
		VkSampler      m_textureSampler;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;

		// parameters
		int                   m_max_frames_in_flight;
		VmaAllocator          m_allocator;
		VkSurfaceKHR          m_surface;
		VkFormat              m_swapChainImageFormat;
		VkPhysicalDevice      m_physicalDevice;
		VkDevice              m_device;
		VkExtent2D            m_swapChainExtent;
		VkSampleCountFlagBits m_msaaSamples;
		VkRenderPass          m_renderPass;
	};

}