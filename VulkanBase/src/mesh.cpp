#include "mesh.h"

using namespace vu;

void Mesh::Destroy(const RendererInfo &rendererInfo) {
	vmaDestroyBuffer(rendererInfo.allocator, m_vertexBuffer, m_vertexAllocation);
	vmaDestroyBuffer(rendererInfo.allocator, m_indexBuffer, m_indexAllocation);
}

void Mesh::BindAndRender(VkCommandBuffer commandBuffer) {
	// bind vertex buffer
	VkBuffer vertexBuffers[] = {m_vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	// bind index buffer
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	// draw
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}


void Mesh::CreateVertexBuffer(const RendererInfo &rendererInfo) {
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	// staging buffer that is visible to cpu
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo stagingAllocationInfo;

	vu::createBuffer (
		rendererInfo.physicalDevice,
		rendererInfo.allocator,
		rendererInfo.surface,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		stagingBuffer, stagingAllocation, stagingAllocationInfo
	);

	// map gpu memory to cpu memory (can access gpu memory like normal)
	vmaCopyMemoryToAllocation(rendererInfo.allocator, m_vertices.data(), stagingAllocation, 0, bufferSize);

	// vertex buffer that is not visible to cpu (faster local gpu memory)
	vu::createBuffer (
		rendererInfo.physicalDevice,
		rendererInfo.allocator,
		rendererInfo.surface,
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO,
		0,
		m_vertexBuffer, m_vertexAllocation, m_vertexAllocationInfo
	);

	// move data from staging buffer to high performance vertex buffer
	vu::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize, rendererInfo.device, rendererInfo.transferCommandPool, rendererInfo.transferQueue);

	// free staging buffer
	vmaDestroyBuffer(rendererInfo.allocator, stagingBuffer, stagingAllocation);
}


void Mesh::CreateIndexBuffer(const RendererInfo &rendererInfo) {
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	// staging buffer that is visible to cpu
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo stagingAllocationInfo;
	vu::createBuffer (
		rendererInfo.physicalDevice,
		rendererInfo.allocator,
		rendererInfo.surface,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		stagingBuffer, stagingAllocation, stagingAllocationInfo
	);

	// map gpu memory to cpu memory (can access gpu memory like normal)
	vmaCopyMemoryToAllocation(rendererInfo.allocator, m_indices.data(), stagingAllocation, 0, bufferSize);

	// index buffer that is not visible to cpu (faster local gpu memory)
	vu::createBuffer (
		rendererInfo.physicalDevice,
		rendererInfo.allocator,
		rendererInfo.surface,
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO,
		0,
		m_indexBuffer, m_indexAllocation, m_indexAllocationInfo
	);

	// move data from staging buffer to high performance index buffer
	vu::copyBuffer(stagingBuffer, m_indexBuffer, bufferSize, rendererInfo.device, rendererInfo.transferCommandPool, rendererInfo.transferQueue);

	// free staging buffer
	vmaDestroyBuffer(rendererInfo.allocator, stagingBuffer, stagingAllocation);
}


void Mesh::LoadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_modelPath.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<vu::Vertex, uint32_t> uniqueVertices{};

	for (const auto &shape : shapes) {
		for (const auto &index : shape.mesh.indices) {
			vu::Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0 - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}

			m_indices.push_back(uniqueVertices[vertex]);
		}
	}
}