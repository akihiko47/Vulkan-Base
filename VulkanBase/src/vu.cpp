#include "vu.h"


void vu::createBuffer(
		VkPhysicalDevice         physicalDevice,
		VmaAllocator             allocator,
		VkSurfaceKHR             surface,
		VkDeviceSize             size,
		VkBufferUsageFlags       bufferUsage,
		VmaMemoryUsage           allocationUsage,
		VmaAllocationCreateFlags allocationFlags,
		VkBuffer                 &buffer,
		VmaAllocation            &allocation,
		VmaAllocationInfo        &allocationInfo) {

	vu::QueueFamilyIndices indices = vu::findQueueFamilies(physicalDevice, surface);
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

std::vector<char> vu::readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();

	return buffer;
}

void vu::preprocessShader(ShaderCompilationInfo &info) {
	std::cout << "preprocessing shader " << info.fileName << "\n";

	shaderc::Compiler compiler;

	// preprocessed info is stored in result
	shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(info.source.data(), info.source.size(), info.kind, info.fileName, info.options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cout << result.GetErrorMessage() << "\n\n";
		throw std::runtime_error("failed to preprocess shader!");
	}

	// copy from result to info struct
	const char *src = result.cbegin();
	size_t newSize = result.end() - src;
	info.source.resize(newSize);
	memcpy(info.source.data(), src, newSize);

	// print preprocessed code
	/*std::string code = {info.source.data(), info.source.data() + info.source.size()};
	std::cout << "preprocessed code:\n";
	std::cout << code << "\n";*/
}

void vu::compileShaderToAssembly(ShaderCompilationInfo &info) {
	std::cout << "compiling shader " << info.fileName << " to assembly\n";

	shaderc::Compiler compiler;

	// preprocessed info is stored in result
	shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(info.source.data(), info.source.size(), info.kind, info.fileName, info.options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cout << result.GetErrorMessage() << "\n\n";
		throw std::runtime_error("failed to compile shader!");
	}

	// copy from result to info struct
	const char *src = result.cbegin();
	size_t newSize = result.end() - src;
	info.source.resize(newSize);
	memcpy(info.source.data(), src, newSize);

	// print compiled code
	/*std::string code = {info.source.data(), info.source.data() + info.source.size()};
	std::cout << "compiled assembly code:\n";
	std::cout << code << "\n";*/
}

void vu::compileShaderToSPIRV(ShaderCompilationInfo &info) {
	std::cout << "compiling shader " << info.fileName << " to SPIR-V\n";

	shaderc::Compiler compiler;

	// preprocessed info is stored in result
	shaderc::SpvCompilationResult result = compiler.AssembleToSpv(info.source.data(), info.source.size(), info.options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cout << result.GetErrorMessage() << "\n\n";
		throw std::runtime_error("failed to compile shader!");
	}

	// copy from result to info struct
	const char *src = reinterpret_cast<const char*>(result.cbegin());
	size_t newSize = reinterpret_cast<const char*>(result.end()) - src;
	info.source.resize(newSize);
	memcpy(info.source.data(), src, newSize);

	// print compiled code
	/*std::string code = {info.source.data(), info.source.data() + info.source.size()};
	std::cout << "compiled SPIR-V code:\n";
	std::cout << code << "\n";*/
}

VkShaderModule vu::createShaderModule(VkDevice device, vu::ShaderCompilationInfo info) {
	vu::preprocessShader(info);
	vu::compileShaderToAssembly(info);
	vu::compileShaderToSPIRV(info);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = info.source.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(info.source.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkFormat vu::findDepthFormat(VkPhysicalDevice physicalDevice) {
	return vu::findSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		physicalDevice
	);
}

// depth image can support multiple formats
	// we can go with VK_FORMAT_D32_SFLOAT but to add flexibility we create this function
	// it takes a list of candidate formats in order from most desirable to least desirable, 
	// and checks which is the first one that is supported by physical device
	// the support of a format depends on the tiling mode and usage
VkFormat vu::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice) {
	for (VkFormat format : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}
