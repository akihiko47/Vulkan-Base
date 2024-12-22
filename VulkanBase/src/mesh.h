#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <tinyobjloader/tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

#include "vulkanUtils.h"

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

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


class Mesh {
public:
	void Load(
		const std::string &modelPath,
		VmaAllocator      &allocator,
		VkDevice          &device,
		VkPhysicalDevice  &physicalDevice,
		VkSurfaceKHR      &surface,
		VkCommandPool     &copyCommandPool,
		VkQueue           &copySubmitQueue
	);
	void Destroy();

	void BindAndRender(VkCommandBuffer commandBuffer);

	Vertex   *GetVertices() { return m_vertices.data(); }
	uint32_t *GetIndices()  { return m_indices.data();  }

private:
	void LoadModel();
	void CreateVertexBuffer();
	void CreateIndexBuffer();

	VmaAllocator     m_allocator;
	VkDevice         m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR     m_surface;
	VkCommandPool    m_copyCommandPool;
	VkQueue          m_copySubmitQueue;

	std::string           m_modelPath;
	std::vector<Vertex>   m_vertices;
	std::vector<uint32_t> m_indices;

	VkBuffer          m_vertexBuffer;
	VmaAllocation     m_vertexAllocation;
	VmaAllocationInfo m_vertexAllocationInfo;
	VkBuffer          m_indexBuffer;
	VmaAllocation     m_indexAllocation;
	VmaAllocationInfo m_indexAllocationInfo;
};


