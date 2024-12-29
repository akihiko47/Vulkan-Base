#pragma once

#include "renderer.h"


using namespace vu;

void Renderer::Run() {
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

void Renderer::InitVulkan() {
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateMemoryAllocator();
	CreateSwapChain();
	CreateImageViews();
	CreateCommandPool();
	CreateRenderPass();
	CreateDepthResources();
	CreateColorResources();
	CreateFrameBuffers();

	CreateTextureImages();
	CreateTextureImageViews();

	CreateTextureSampler();
	CreateShaderModules();

	CreateDescriptorSetLayout();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateGraphicsPipeline();
			

	mesh1.Load("models/viking_room.obj", allocator, device, physicalDevice, surface, transferCommandPool, transferQueue);
	mesh2.Load("models/tree.obj", allocator, device, physicalDevice, surface, transferCommandPool, transferQueue);
	transform1 = vu::Transform(glm::vec3(0.0, 0.0, 0.0));
	transform2 = vu::Transform(glm::vec3(2.0, 0.0, 0.0));
	camTransform = vu::Transform(glm::vec3(0.0, 0.0, 0.0));
	CreateCommandBuffers();
	CreateSyncObjects();
}

void Renderer::MainLoop() {
	while (!glfwWindowShouldClose(m_window)) {
		ProcessInput();
		UpdateTime();

		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(device);
}

void Renderer::Cleanup() {
	CleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);

	vkDestroyImageView(device, textureImageView1, nullptr);
	vmaDestroyImage(allocator, textureImage1, textureImageAllocation1);

	vkDestroyImageView(device, textureImageView2, nullptr);
	vmaDestroyImage(allocator, textureImage2, textureImageAllocation2);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(allocator, uniformBuffers[i], uniformAllocations[i]);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);  // destroys descriptor sets as well
	vkDestroyDescriptorSetLayout(device, descriptorSetLayoutGlobal, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayoutLocal, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		
	destroyShaderModules();

	mesh1.Destroy();
	mesh2.Destroy();

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);  // destroys cammand buffers as well
	vkDestroyCommandPool(device, transferCommandPool, nullptr);  // destroys cammand buffers as well

	vmaDestroyAllocator(allocator);

	vkDestroyDevice(device, nullptr);
		
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
	}
		
	vkDestroySurfaceKHR(m_instance, surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::ProcessInput() {
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(m_window, true);
	}

	float currentCamSpeed = camSpeed;
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		currentCamSpeed *= 5.0f;
	}

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		camTransform.SetPosition(camTransform.GetPosition() + camTransform.GetForward() * currentCamSpeed * deltaTime);
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		camTransform.SetPosition(camTransform.GetPosition() - camTransform.GetForward() * currentCamSpeed * deltaTime);
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		camTransform.SetPosition(camTransform.GetPosition() - camTransform.GetRight() * currentCamSpeed * deltaTime);
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		camTransform.SetPosition(camTransform.GetPosition() + camTransform.GetRight() * currentCamSpeed * deltaTime);
	}
}

void Renderer::SendMouseCallbackToInstance(GLFWwindow* window, double xpos, double ypos) {
	if (Renderer *renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window))) {
		renderer->ProcessMouseInput(xpos, ypos);
	}
}

void Renderer::ProcessMouseInput(double xpos, double ypos) {
	float offsetX = xpos - lastX;
	float offsetY = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	offsetY *= sensitivity;
	offsetX *= sensitivity;

	camTransform.RotateX(offsetY);
	camTransform.RotateY(offsetX);
}

void Renderer::InitWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // do not create OpenGL context
	// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // disable resizing

	// width, height, title, monitor, something from OpenGL
	m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

	// focus mouse and hide cursor (like fps)
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(m_window, SendMouseCallbackToInstance);
}

void Renderer::CreateInstance() {
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Optional, but good
	VkApplicationInfo appInfo = VkApplicationInfo();
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.pNext = nullptr;

	// === Create instance info === 
	VkInstanceCreateInfo createInfo = VkInstanceCreateInfo();
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// extension to interface with window system (GLFW has function that gets that)
	std::vector<const char*> extensions = GetRequiredExtensions();
	std::cout << "required instance extensions:\n";
	for (int i = 0; i < extensions.size(); i++) {
		std::cout << '\t' << extensions[i] << "\n";
	}
	std::cout << "\n";

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::cout << "available instance extensions:\n";
	for (int i = 0; i < availableExtensions.size(); i++) {
		std::cout << '\t' << availableExtensions[i].extensionName << "\n";
	}
	std::cout << "\n";

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// validation layers to enable
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = VkDebugUtilsMessengerCreateInfoEXT();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

}

void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 	VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr; // Optional
}

void Renderer::SetupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = VkDebugUtilsMessengerCreateInfoEXT();
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void Renderer::CreateSurface() {
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void Renderer::PickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	std::cout << "physical graphics devices:\n";
	for (VkPhysicalDevice device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		std::cout << "\t" << deviceProperties.deviceName << "\n";
	}

	for (const VkPhysicalDevice &device : devices) {
		if (IsDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples = GetMaxUsableSampleCount();
			std::cout << "\nMSAA samples count: " << msaaSamples << "\n\n";
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Renderer::CreateLogicalDevice() {
	vu::QueueFamilyIndices indices = vu::findQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = VkDeviceQueueCreateInfo();
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
		

	VkPhysicalDeviceFeatures deviceFeatures = VkPhysicalDeviceFeatures();
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	// === Main info ===
	VkDeviceCreateInfo createInfo = VkDeviceCreateInfo();
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	std::cout << "required logical device extensions:\n";
	for (int i = 0; i < deviceExtensions.size(); i++) {
		std::cout << '\t' << deviceExtensions[i] << "\n";
	}
	std::cout << "\n";

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
}

bool Renderer::IsDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	vu::QueueFamilyIndices indices = vu::findQueueFamilies(device, surface);
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);

	bool deviceSupporstNeededQueues = indices.isComplete();
	bool deviceExtensionSupported = CheckDeviceExtensionSupport(device);
	bool deviceSupportsSwapChain = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

	bool suitable = deviceSupporstNeededQueues && deviceExtensionSupported && deviceSupportsSwapChain && deviceFeatures.samplerAnisotropy;

	std::cout << "\nphysical device " << deviceProperties.deviceName << " is " << (suitable ? "suitable" : "NOT suitable") << "\n\n";

	return suitable;
}

bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::cout << "required physical device extensions:\n";
	for (int i = 0; i < deviceExtensions.size(); i++) {
		std::cout << '\t' << deviceExtensions[i] << "\n";
	}
	std::cout << "\n";

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void Renderer::CreateMemoryAllocator() {
	VmaAllocatorCreateInfo createInfo{};
	createInfo.instance = m_instance;
	createInfo.physicalDevice = physicalDevice;
	createInfo.device = device;
	createInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	vmaCreateAllocator(&createInfo, &allocator);
}

// This function returns swap chain settings that are supported by physical device and surface
// We store these settings in a struct (can find at the beginning of file)
// Later we will choose best settings among them and create swap chain
SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void Renderer::CreateSwapChain() {
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;  // minimum is small, add 1
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;  // cant have more than maximum
	}

	// === Main info ===
	VkSwapchainCreateInfoKHR createInfo = VkSwapchainCreateInfoKHR();
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;  // amount of layers each image consists of (for stereoscopic 3D application)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vu::QueueFamilyIndices indices = vu::findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {  // most of hardware
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	// specify that we dont need any transformations (rotation etc.)
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// dont use alpha for blending with other windows in system
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
		
	// enable clipping with other windows in system
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// get images from swap chain
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

// choose best surface format among formats that are supported by physical devise and surface
VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats) {
	for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	std::cout << "unable to find needed image format, returning first format that device supports\n\n";
	return availableFormats[0];
}

// choose best present mode among formats that are supported by physical devise and surface
VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	std::cout << "unable to find needed present mode, swap chain will use regular queue\n\n";
	//return VK_PRESENT_MODE_FIFO_KHR;    // vsync ON
	return VK_PRESENT_MODE_IMMEDIATE_KHR; // vsync OFF
}

// setup swap chain images resolution
VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkImageView Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
	VkImageViewCreateInfo createInfo = VkImageViewCreateInfo();
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;

	// can fill with 1 or 0 or swizzle channels around (here uses default)
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}
	return imageView;
}

// To use images we need to create VkImageView objects from them
void Renderer::CreateImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	for (int i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

// Returns extensions that are needed for creating Instance
// These extensions are platform-specific but GLFW has function that return extension needed for creating window
// We also add extension for validation layers if needed
std::vector<const char*> Renderer::GetRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// This function finds out if needed validation layers (validationLayers vector at beginning of document)
// Are supported by Instance (availableLayers vector that is created in this function)
bool Renderer::CheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "Available validation layers:" << "\n";
	for (const auto& layerProperties : availableLayers) {
		std::cout << "\t" << layerProperties.layerName << "\n";
	}
	std::cout << "\n";

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void Renderer::CreateDescriptorSetLayout() {

	// SET 0
	// view + projection matrix
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 1> bindingsGlobal = {uboLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfoGlobal{};
	layoutInfoGlobal.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfoGlobal.bindingCount = static_cast<uint32_t>(bindingsGlobal.size());
	layoutInfoGlobal.pBindings = bindingsGlobal.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfoGlobal, nullptr, &descriptorSetLayoutGlobal) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}


	// SET 1
	// material specific bindings
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 1> bindingsLocal = {samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfoLocal{};
	layoutInfoLocal.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfoLocal.bindingCount = static_cast<uint32_t>(bindingsLocal.size());
	layoutInfoLocal.pBindings = bindingsLocal.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfoLocal, nullptr, &descriptorSetLayoutLocal) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl << std::endl;

	return VK_FALSE;  // stop code that triggered that callback?
}

void Renderer::FramebufferResizeCallback(GLFWwindow *window, int width, int height) {
	auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Renderer::CreateRenderPass() {
	// create attachments

	// multisampled color attachment to enable msaa
	// this attachiment cant be presented
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;  // for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // color and depth
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // color and depth
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // stencil
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // stencil
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// depth attachment
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = vu::findDepthFormat(physicalDevice);
	depthAttachment.samples = msaaSamples;  // for multisampling
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // color and depth
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // color and depth
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // stencil
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // stencil
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// color attachment that will be presented
	// multisampled image will be resolved to this regular image
	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;  // for multisampling
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // color and depth
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // color and depth
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // stencil
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // stencil
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// create attachments
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// create subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;  // not compute subpass
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	// we need subpass dependencies to synchronize subpasses
	// this subpass dependencie tells that we need to wait for previous render pass
	// (wait for all of the subpasses within all of the render passes before this one)
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;  // index of subpass that we are dependent on
	dependency.dstSubpass = 0;                    // index of current subpass
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;  // wait untill these stages are finished
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;  // stages that are NOT allowed to execute before srcStageMask stage is finished
	dependency.srcAccessMask = 0;  // what types of memory access src subpass uses
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;  // what types we will be using in current subpass

	// create render pass
	std::array< VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void Renderer::CreateFrameBuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Renderer::CreateCommandPool() {
	vu::QueueFamilyIndices queueFamilyIndices = vu::findQueueFamilies(physicalDevice, surface);

	VkCommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // allow buffers to be reset individually
	graphicsPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &graphicsPoolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}

	VkCommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	transferPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	if (vkCreateCommandPool(device, &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create transfer command pool!");
	}
}

void Renderer::CreateCommandBuffers() {
	graphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo graphicsAllocInfo{};
	graphicsAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	graphicsAllocInfo.commandPool = graphicsCommandPool;
	graphicsAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	graphicsAllocInfo.commandBufferCount = (uint32_t)graphicsCommandBuffers.size();

	if (vkAllocateCommandBuffers(device, &graphicsAllocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate graphics command buffers!");
	}


	transferCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo transferAllocInfo{};
	transferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	transferAllocInfo.commandPool = transferCommandPool;
	transferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	transferAllocInfo.commandBufferCount = (uint32_t)transferCommandBuffers.size();

	if (vkAllocateCommandBuffers(device, &transferAllocInfo, transferCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate transfer command buffers!");
	}
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

	// begin recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// start render pass
	std::array<VkClearValue, 3> clearValues{};  // same as attachments order
	clearValues[0].color = {{0.17f, 0.12f, 0.19f, 1.0f}};  // resolve image color
	clearValues[1].depthStencil = {1.0f, 0};
	clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};  // multisampled image

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// update viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	// update scissor rect
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// bind pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	SetGlobalPushConstants(commandBuffer);

	// bind global descriptors
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetsGlobal.data()[currentFrame], 0, nullptr);

		// bind material 1 descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSetsMat1.data()[currentFrame], 0, nullptr);

			// bind model matrix constants and render
			transform1.BindModelMatrix(commandBuffer, pipelineLayout);
			mesh1.BindAndRender(commandBuffer);


		// bind material 2 descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSetsMat2.data()[currentFrame], 0, nullptr);

			//material2.BindMaterial(commandBuffer, currentFrame, pushConstants);
			transform2.BindModelMatrix(commandBuffer, pipelineLayout);
			mesh2.BindAndRender(commandBuffer);

	// end render pass
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Renderer::CreateGraphicsPipeline() {

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	// these values can be changed at runtime without recreating pipeline
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	// vertex input
	VkVertexInputBindingDescription bindingDescription = vu::Vertex::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = vu::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// what type of geometry will be drawn
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// region of framebuffer to render to
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// discard pixels outside this area
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	// create viewport using viewport and scissor rect from earlier
	// creating multiple require feature in logical device
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// setup resterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;                // clamp instead of discarding fragments when depth testing (need gpu feature)
	rasterizer.rasterizerDiscardEnable = VK_FALSE;                // disable output to future stages 
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;    // can be lines or points (require gpu feature)
	rasterizer.lineWidth = 1.0f;                    // more that one - need wideLines gpu feature
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;  // culling
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // triangle front direction
	rasterizer.depthBiasEnable = VK_FALSE;                // bias for shadow mapping
	rasterizer.depthBiasConstantFactor = 0.0f;                    // optional
	rasterizer.depthBiasClamp = 0.0f;                    // optional
	rasterizer.depthBiasSlopeFactor = 0.0f;                    // optional

	// multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = 0.2f;
	multisampling.rasterizationSamples = msaaSamples;

	// color blending (now its alpha blending) per framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	/*
	if (blendEnable) {
		finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
		finalColor.a   = (srcAlphaBlendFactor * newColor.a)   <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	} else {
		finalColor = newColor;
	}

	finalColor = finalColor & colorWriteMask;
	*/

	// color blending global
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;         // set gloabl logical operation for blending (disables local attachments)
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // logical operation here
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// push constants
	std::array<VkPushConstantRange, 2> pushConstants{};
	pushConstants[0].offset = 0;
	pushConstants[0].size = 64;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pushConstants[1].offset = 64;
	pushConstants[1].size = 64;
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// pipeline layout (descriptors and push constants)
	std::array<VkDescriptorSetLayout, 2> descriptorLayouts {descriptorSetLayoutGlobal, descriptorSetLayoutLocal};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// depth testing
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;  // range of depths
	depthStencil.stencilTestEnable = VK_FALSE;

	// create pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void Renderer::CreateDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 3);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Renderer::CreateUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(vu::VPubo);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformAllocations.resize(MAX_FRAMES_IN_FLIGHT);
	uniformAllocationInfos.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vu::createBuffer(
			physicalDevice,
			allocator,
			surface,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			uniformBuffers[i],
			uniformAllocations[i],
			uniformAllocationInfos[i]
		);
	}
}

void Renderer::CreateDescriptorSets() {
	// allocate descriptor sets
	std::vector<VkDescriptorSetLayout> layoutsGlobal{MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutGlobal};
	std::vector<VkDescriptorSetLayout> layoutsMat1{MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutLocal};
	std::vector<VkDescriptorSetLayout> layoutsMat2{MAX_FRAMES_IN_FLIGHT, descriptorSetLayoutLocal};

	// global
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layoutsGlobal.data();

	descriptorSetsGlobal.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsGlobal.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	// material 1
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layoutsMat1.data();

	descriptorSetsMat1.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsMat1.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	// material 2
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layoutsMat2.data();

	descriptorSetsMat2.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsMat2.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}


	// populate sets with data
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(vu::VPubo);

		VkDescriptorImageInfo imageInfo1{};
		imageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo1.imageView = textureImageView1;
		imageInfo1.sampler = textureSampler;

		VkDescriptorImageInfo imageInfo2{};
		imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo2.imageView = textureImageView2;
		imageInfo2.sampler = textureSampler;

		// global descriptor (VP matrix)
		std::array<VkWriteDescriptorSet, 1> descriptorWritesGlobal{};
		descriptorWritesGlobal[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWritesGlobal[0].dstSet = descriptorSetsGlobal[i];
		descriptorWritesGlobal[0].dstBinding = 0;
		descriptorWritesGlobal[0].dstArrayElement = 0;
		descriptorWritesGlobal[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWritesGlobal[0].descriptorCount = 1;
		descriptorWritesGlobal[0].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWritesGlobal.size()), descriptorWritesGlobal.data(), 0, nullptr);

		// local descriptor (images and such) material 1
		std::array<VkWriteDescriptorSet, 1> descriptorWritesLocal1{};
		descriptorWritesLocal1[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWritesLocal1[0].dstSet = descriptorSetsMat1[i];
		descriptorWritesLocal1[0].dstBinding = 0;
		descriptorWritesLocal1[0].dstArrayElement = 0;
		descriptorWritesLocal1[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWritesLocal1[0].descriptorCount = 1;
		descriptorWritesLocal1[0].pImageInfo = &imageInfo1;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWritesLocal1.size()), descriptorWritesLocal1.data(), 0, nullptr);

		// local descriptor (images and such) material 1
		std::array<VkWriteDescriptorSet, 1> descriptorWritesLocal2{};
		descriptorWritesLocal2[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWritesLocal2[0].dstSet = descriptorSetsMat2[i];
		descriptorWritesLocal2[0].dstBinding = 0;
		descriptorWritesLocal2[0].dstArrayElement = 0;
		descriptorWritesLocal2[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWritesLocal2[0].descriptorCount = 1;
		descriptorWritesLocal2[0].pImageInfo = &imageInfo2;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWritesLocal2.size()), descriptorWritesLocal2.data(), 0, nullptr);
	}
}


void Renderer::CleanupSwapChain() {
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vmaDestroyImage(allocator, depthImage, depthImageAllocation);

	vkDestroyImageView(device, colorImageView, nullptr);
	vmaDestroyImage(allocator, colorImage, colorImageAllocation);
}

void Renderer::RecreateSwapChain() {
	int width, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0) {
		if (glfwWindowShouldClose(m_window)) {
			return;
		}
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateDepthResources();
	CreateColorResources();
	CreateFrameBuffers();
}

void Renderer::CreateShaderModules() {
	vu::ShaderCompilationInfo vertShaderInfo{};
	vertShaderInfo.fileName = "shaders/shader.vert";
	vertShaderInfo.source = vu::readFile(vertShaderInfo.fileName);
	vertShaderInfo.kind = shaderc_vertex_shader;
	vertShaderInfo.options.SetOptimizationLevel(shaderc_optimization_level_performance);

	vu::ShaderCompilationInfo fragShaderInfo{};
	fragShaderInfo.fileName = "shaders/shader.frag";
	fragShaderInfo.source = vu::readFile(fragShaderInfo.fileName);
	fragShaderInfo.kind = shaderc_fragment_shader;
	fragShaderInfo.options.SetOptimizationLevel(shaderc_optimization_level_performance);

	vertShaderModule = vu::createShaderModule(device, vertShaderInfo);
	fragShaderModule = vu::createShaderModule(device, fragShaderInfo);
}

void Renderer::destroyShaderModules() {
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}
		

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	// need this to see available memory types of physical device
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		// if this memory type is needed type AND memory type has all properties we need
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
}

void Renderer::CreateSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
	VkCommandBuffer commandBuffer = vu::beginSingleTimeCommands(transferCommandPool, device);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // we dont transfer queue ownership
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // we dont transfer queue ownership
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer,
							sourceStage, destinationStage,
							0,
							0, nullptr,
							0, nullptr,
							1, &barrier);

	vu::endSingleTimeCommands(commandBuffer, transferCommandPool, device, transferQueue);
}

void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = vu::beginSingleTimeCommands(transferCommandPool, device);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vu::endSingleTimeCommands(commandBuffer, transferCommandPool, device, transferQueue);
}

void Renderer::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	// check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBufferGraphics = vu::beginSingleTimeCommands(graphicsCommandPool, device);

	// we need barrier to transition mip levels layouts to copy from and to them
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {

		// set i-1 level image to src
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBufferGraphics,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// blit from i-1 to i
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBufferGraphics,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		// transition i-1 level to shader read only
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBufferGraphics,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// decrease image size
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// transition last mip level to shader read only
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBufferGraphics,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vu::endSingleTimeCommands(commandBufferGraphics, graphicsCommandPool, device, transferQueue);
}

void Renderer::CreateTextureImages() {
	CreateTextureImage("textures/viking_room.png", mipLevels1, textureImage1, textureImageAllocation1, textureImageAllocationInfo1);
	CreateTextureImage("textures/gradient.png", mipLevels2, textureImage2, textureImageAllocation2, textureImageAllocationInfo2);
}

void Renderer::CreateTextureImageViews() {
	textureImageView1 = CreateImageView(textureImage1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels1);
	textureImageView2 = CreateImageView(textureImage2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels2);
}

void Renderer::CreateTextureImage(const std::string &imagePath, uint32_t &mipLevels, VkImage &image, VmaAllocation &textureImageAllocation, VmaAllocationInfo &textureImageAllocationInfo) {
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	// staging buffer that is visible to cpu
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo stagingAllocationInfo;
	vu::createBuffer(
		physicalDevice,
		allocator,
		surface,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		stagingBuffer, stagingAllocation, stagingAllocationInfo
	);

	// copy pixels to staging buffer
	vmaCopyMemoryToAllocation(allocator, pixels, stagingAllocation, 0, imageSize);

	// free pixels array
	stbi_image_free(pixels);

	// create image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(texWidth);
	imageInfo.extent.height = static_cast<uint32_t>(texHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // linear if we need to access memory directly (for staging image)
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // preinitialized if we need to use this image as staging image
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // used by one queue family (transfer)
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;  // for multisampling
	imageInfo.flags = 0;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

	vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &textureImageAllocation, &textureImageAllocationInfo);

	// copy staging buffer to image
		
	// undefined -> transfer
	TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

	// copy
	CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	// generate mip-maps (will transition layout transfer -> shader)
	GenerateMipmaps(image, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);

	// destroy staging buffer
	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

		
void Renderer::CreateTextureSampler() {
	VkPhysicalDeviceProperties deviceProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;  // magnification
	samplerInfo.minFilter = VK_FILTER_LINEAR;  // minification
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;  // color when sampling beyound image
	samplerInfo.unnormalizedCoordinates = VK_FALSE;  // use [0, 1) ranges and not [0, texWidth)
	samplerInfo.compareEnable = VK_FALSE;            // for percentage-closer filtering on shadow maps
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;    // for percentage-closer filtering on shadow maps
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void Renderer::CreateDepthResources() {
	// choose depth image format
	VkFormat depthFormat = vu::findDepthFormat(physicalDevice);

	// create depth image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = swapChainExtent.width;
	imageInfo.extent.height = swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = depthFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // linear if we need to access memory directly (for staging image)
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // preinitialized if we need to use this image as staging image
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // used by one queue family (transfer)
	imageInfo.samples = msaaSamples;  // for multisampling
	imageInfo.flags = 0;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthImage, &depthImageAllocation, &depthImageAllocationInfo);

	// create image view
	depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	// change layout so we can access image in shaders (here it is optional)
	TransitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

// create special buffer for multisampled image
// we need this for MSAA
void Renderer::CreateColorResources() {
	// choose color image format
	VkFormat colorFormat = swapChainImageFormat;

	// create color image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = swapChainExtent.width;
	imageInfo.extent.height = swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = colorFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // linear if we need to access memory directly (for staging image)
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // preinitialized if we need to use this image as staging image
	imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // used by one queue family (transfer)
	imageInfo.samples = msaaSamples;  // for multisampling
	imageInfo.flags = 0;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

	vmaCreateImage(allocator, &imageInfo, &allocInfo, &colorImage, &colorImageAllocation, &colorImageAllocationInfo);

	// create image view for this image
	colorImageView = CreateImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

	

bool Renderer::HasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

	

VkSampleCountFlagBits Renderer::GetMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	// depth and color has different maximum sample count
	// we will use one that is smaller (and supported by physical device)
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void Renderer::UpdateTime() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	currentFrameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	deltaTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;
}

void Renderer::UpdateTransforms() {
	transform1.SetRotation(glm::vec3(0.0f, currentFrameTime * glm::radians(90.0f) * 0.2f, 0.0f));
	transform1.SetScale(glm::vec3(1.0f, glm::sin(currentFrameTime * 2.0) * 0.3 + 0.7, 1.0));
}

void Renderer::SetGlobalUniformBuffers(uint32_t currentImage) {
	glm::mat4 view = glm::lookAt(camTransform.GetPosition(), camTransform.GetPosition() + camTransform.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);

	vu::VPubo ubo{};
	ubo.view = view;
	ubo.proj = proj;
	ubo.proj[1][1] *= -1;

	memcpy(uniformAllocationInfos[currentImage].pMappedData, &ubo, sizeof(ubo));
}

void Renderer::SetGlobalPushConstants(VkCommandBuffer commandBuffer) {
	vu::PushConstantsGlobal pushConstants{};
	pushConstants.data1 = glm::vec4(
		currentFrameTime,
		camTransform.GetPosition().x,
		camTransform.GetPosition().y,
		camTransform.GetPosition().z
	);

	// bind global push constants
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64, &pushConstants);
}

void Renderer::DrawFrame() {
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// get image from swap chain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		std::cout << "recreating swap chain\n\n";
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// reset fence only if we are submitting work
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// update uniform buffers of materials
	SetGlobalUniformBuffers(currentFrame);

	// update objects positions
	UpdateTransforms();

	// record commands to command buffer
	vkResetCommandBuffer(graphicsCommandBuffers[currentFrame], 0);
	RecordCommandBuffer(graphicsCommandBuffers[currentFrame], imageIndex);

	// submit command buffer
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &graphicsCommandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// submitting result to swap chain
	VkSwapchainKHR swapChains[] = {swapChain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		RecreateSwapChain();
		std::cout << "recreating swap chain\n\n";
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// advance to the next frame
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

