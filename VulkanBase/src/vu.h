#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

#include <stdexcept>
#include <optional>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>


namespace vu {

	struct PushConstants {
		glm::vec4 data;  // x - time, yzw - camPos
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	// Need this struct for vertex buffer
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;  // start
			bindingDescription.stride = sizeof(Vertex);  // step (32 bytes)
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // per vertex (other setting for instancing)

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			// position
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);  // 0 bytes

			// normal
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);  // 12 bytes

			// UV coordinates
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);  // 24 bytes

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
		}
	};

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

namespace std {
	template<> struct hash<vu::Vertex> {
		size_t operator()(vu::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
