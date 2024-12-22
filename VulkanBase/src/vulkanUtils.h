#pragma once
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>

#include <stdexcept>
#include <optional>
#include <vector>

namespace vu {

	void createBuffer (
		VkPhysicalDevice         physicalDevice,
		VmaAllocator             allocator,
		VkSurfaceKHR             surface,
		VkDeviceSize             size,
		VkBufferUsageFlags       bufferUsage,
		VmaMemoryUsage           allocationUsage,
		VmaAllocationCreateFlags allocationFlags,
		VkBuffer                 &buffer,
		VmaAllocation            &allocation,
		VmaAllocationInfo        &allocationInfo
	);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue submitQueue);

	VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkDevice device, VkQueue submitQueue);

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}
