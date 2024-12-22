#include "mesh.h"

void Mesh::Load(
		const std::string &modelPath,
		VmaAllocator      &allocator,
		VkDevice          &device,
		VkPhysicalDevice  &physicalDevice,
		VkSurfaceKHR      &surface,
		VkCommandPool     &copyCommandPool,
		VkQueue           &copySubmitQueue){

	m_modelPath       = modelPath;
	m_allocator       = allocator;
	m_device          = device;
	m_physicalDevice  = physicalDevice;
	m_surface         = surface;
	m_copyCommandPool = copyCommandPool;
	m_copySubmitQueue = copySubmitQueue;

	LoadModel();
	CreateVertexBuffer();
	CreateIndexBuffer();
}


void Mesh::Destroy() {
	vmaDestroyBuffer(m_allocator, m_vertexBuffer, m_vertexAllocation);
	vmaDestroyBuffer(m_allocator, m_indexBuffer, m_indexAllocation);
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


void Mesh::CreateVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	// staging buffer that is visible to cpu
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo stagingAllocationInfo;

	vu::createBuffer (
		m_physicalDevice,
		m_allocator,
		m_surface,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		stagingBuffer, stagingAllocation, stagingAllocationInfo
	);

	// map gpu memory to cpu memory (can access gpu memory like normal)
	vmaCopyMemoryToAllocation(m_allocator, m_vertices.data(), stagingAllocation, 0, bufferSize);

	// vertex buffer that is not visible to cpu (faster local gpu memory)
	vu::createBuffer (
		m_physicalDevice,
		m_allocator,
		m_surface,
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO,
		0,
		m_vertexBuffer, m_vertexAllocation, m_vertexAllocationInfo
	);

	// move data from staging buffer to high performance vertex buffer
	vu::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize, m_device, m_copyCommandPool, m_copySubmitQueue);

	// free staging buffer
	vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);
}


void Mesh::CreateIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	// staging buffer that is visible to cpu
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo stagingAllocationInfo;
	vu::createBuffer(
		m_physicalDevice,
		m_allocator,
		m_surface,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		stagingBuffer, stagingAllocation, stagingAllocationInfo
	);

	// map gpu memory to cpu memory (can access gpu memory like normal)
	vmaCopyMemoryToAllocation(m_allocator, m_indices.data(), stagingAllocation, 0, bufferSize);

	// index buffer that is not visible to cpu (faster local gpu memory)
	vu::createBuffer(
		m_physicalDevice,
		m_allocator,
		m_surface, 
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO,
		0,
		m_indexBuffer, m_indexAllocation, m_indexAllocationInfo
	);

	// move data from staging buffer to high performance index buffer
	vu::copyBuffer(stagingBuffer, m_indexBuffer, bufferSize, m_device, m_copyCommandPool, m_copySubmitQueue);

	// free staging buffer
	vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);
}


void Mesh::LoadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_modelPath.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto &shape : shapes) {
		for (const auto &index : shape.mesh.indices) {
			Vertex vertex{};

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