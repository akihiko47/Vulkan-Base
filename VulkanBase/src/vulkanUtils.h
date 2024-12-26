#pragma once

#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

#include <stdexcept>
#include <optional>
#include <vector>
#include <iostream>
#include <fstream>

namespace vu {

	// Need this struct to store information about shaders to compile them
	struct ShaderCompilationInfo {
		const char             *fileName;
		shaderc_shader_kind     kind;
		std::vector<char>       source;
		shaderc::CompileOptions options;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};

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

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	// === SHADERS ===
	std::vector<char> readFile(const std::string &filename);
	void preprocessShader(ShaderCompilationInfo &info);
	void compileShaderToAssembly(ShaderCompilationInfo &info);
	void compileShaderToSPIRV(ShaderCompilationInfo &info);
	VkShaderModule createShaderModule(VkDevice device, vu::ShaderCompilationInfo info);

	// === FORMATS ===
	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
	VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice);
}
