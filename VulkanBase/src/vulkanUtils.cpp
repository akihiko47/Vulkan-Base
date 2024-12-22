#include "vulkanUtils.h"

void vu::createBuffer(
		VkPhysicalDevice physicalDevice,
		VmaAllocator allocator,
		VkSurfaceKHR surface,
		VkDeviceSize size,
		VkBufferUsageFlags bufferUsage,
		VmaMemoryUsage allocationUsage,
		VmaAllocationCreateFlags allocationFlags,
		VkBuffer &buffer,
		VmaAllocation &allocation,
		VmaAllocationInfo &allocationInfo) {

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.transferFamily.value()};

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsage;

	if (indices.graphicsFamily != indices.transferFamily) {
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;  // owned by one queue family or multiple at the same time
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // owned by one queue family or multiple at the same time
	}

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = allocationUsage;
	allocInfo.flags = allocationFlags;

	if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
}


// This function gets queue families supported by physical device
	// And checks if these families are valid for our tasks
	// 
	// This function returs index of valid queue family for each task
	// Packed in structure
	//
	// isComplete() - we found queue families for all needed tasks
vu::QueueFamilyIndices vu::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	/*std::cout << "avaliable queue families:\n";
	for (VkQueueFamilyProperties queueProperties : queueFamilies) {
		std::cout << "\t" << "queues in family: " << queueProperties.queueCount << "\n";
	}
	std::cout << "\n";*/

	for (int i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT & ~VK_QUEUE_GRAPHICS_BIT) {
			indices.transferFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}


void vu::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue submitQueue) {
	VkCommandBuffer commandBuffer = vu::beginSingleTimeCommands(commandPool, device);

	// copy buffers
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vu::endSingleTimeCommands(commandBuffer, commandPool, device, submitQueue);
}


VkCommandBuffer vu::beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device) {
	// create command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	// begin recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // tell driver that this command buffer will be used once

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}


void vu::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkDevice device, VkQueue submitQueue) {
	// stop recording
	vkEndCommandBuffer(commandBuffer);

	// submit command buffer to queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(submitQueue);

	// free command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
