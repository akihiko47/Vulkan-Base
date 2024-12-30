#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include <tinyobjloader/tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>

#include "vu.h"

namespace vu {

	class Renderer;

	class Mesh {
	public:
		Mesh(const RendererInfo &rendererInfo, const std::string &modelPath) : m_modelPath(modelPath) {
			LoadModel();
			CreateVertexBuffer(rendererInfo);
			CreateIndexBuffer(rendererInfo);
		};

		void Destroy(const RendererInfo &rendererInfo);

		void BindAndRender(VkCommandBuffer commandBuffer);

		Vertex   *GetVertices() { return m_vertices.data(); }
		uint32_t *GetIndices()  { return m_indices.data(); }

	private:
		void LoadModel();
		void CreateVertexBuffer(const RendererInfo &rendererInfo);
		void CreateIndexBuffer(const RendererInfo &rendererInfo);

		std::string             m_modelPath;
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t>   m_indices;

		VkBuffer          m_vertexBuffer;
		VmaAllocation     m_vertexAllocation;
		VmaAllocationInfo m_vertexAllocationInfo;
		VkBuffer          m_indexBuffer;
		VmaAllocation     m_indexAllocation;
		VmaAllocationInfo m_indexAllocationInfo;
	};

}
