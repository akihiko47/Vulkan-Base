#pragma once

#include <tinyobjloader/tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

#include "vu.h"

namespace vu {

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

		vu::Vertex   *GetVertices() { return m_vertices.data(); }
		uint32_t *GetIndices() { return m_indices.data(); }

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
		std::vector<vu::Vertex>   m_vertices;
		std::vector<uint32_t> m_indices;

		VkBuffer          m_vertexBuffer;
		VmaAllocation     m_vertexAllocation;
		VmaAllocationInfo m_vertexAllocationInfo;
		VkBuffer          m_indexBuffer;
		VmaAllocation     m_indexAllocation;
		VmaAllocationInfo m_indexAllocationInfo;
	};

}
