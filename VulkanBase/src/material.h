#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <iostream>

#include "vu.h"
#include "transform.h"

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
		void SetUniformBuffer(uint32_t currentImage, vu::Transform modelTransform, glm::mat4 view);
		void UpdateSwapChain(VkExtent2D swapChainExtent);
		void BindMaterial(VkCommandBuffer commandBuffer, uint32_t currentFrame, PushConstants &pushConstants);
		void Destroy();

	private:
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateUniformBuffers();

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

		// creating these variables
		VkDescriptorSetLayout          m_descriptorSetLayout;
		VkPipelineLayout               m_pipelineLayout;
		VkPipeline                     m_graphicsPipeline;
		VkDescriptorPool               m_descriptorPool;
		std::vector<VkBuffer>          m_uniformBuffers;
		std::vector<VmaAllocation>     m_uniformAllocations;
		std::vector<VmaAllocationInfo> m_uniformAllocationInfos;
		std::vector<VkDescriptorSet>   m_descriptorSets;
	};

}