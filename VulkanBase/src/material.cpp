#include "material.h"

using namespace vu;

void Material::Initialize(
		VmaAllocator          allocator,
		VkSurfaceKHR          surface,
		VkFormat              swapChainImageFormat,
		const int             max_frames_in_flight,
		VkPhysicalDevice      physicalDevice,
		VkDevice              device,
		VkExtent2D            swapChainExtent,
		VkSampleCountFlagBits msaaSamples,
		VkRenderPass          renderPass) {

	m_allocator             = allocator;
	m_surface               = surface;
	m_swapChainImageFormat  = swapChainImageFormat;
	m_max_frames_in_flight  = max_frames_in_flight;
	m_physicalDevice        = physicalDevice;
	m_device                = device;
	m_swapChainExtent       = swapChainExtent;
	m_msaaSamples           = msaaSamples;
	m_renderPass            = renderPass;
}


void Material::SetResourses(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, VkImageView textureImageView, VkSampler textureSampler) {
	m_vertShaderModule = vertShaderModule;
	m_fragShaderModule = fragShaderModule;
	m_textureImageView = textureImageView;
	m_textureSampler = textureSampler;
}

