#pragma once

#include "mesh.h"
#include "material.h"

#define VK_LOD_CLAMP_NONE 15.0f  // max mipmap level for sampler
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <VMA/vk_mem_alloc.h>
#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <array>
#include <set>
#include <optional>
#include <limits> 
#include <algorithm>
#include <chrono>
#include <unordered_map>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string TEXTURE_PATH1 = "textures/viking_room.png";
const std::string TEXTURE_PATH2 = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

// All standart layers are packed in this one
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_monitor"
};

// Need this extension to create swapchain
const std::vector<const char*> deviceExtensions = {
	"VK_KHR_swapchain"  // can use VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Enable layers only in debug
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

namespace vu {
	class Renderer {
	public:
		void Run();

		// getters
		VkInstance GetInstance() { return m_instance; }

	private:
		void InitWindow();
		void InitVulkan();
		void MainLoop();
		void DrawFrame();
		void Cleanup();

		// input (should move it from here)
		void ProcessInput();
		static void SendMouseCallbackToInstance(GLFWwindow* window, double xpos, double ypos);
		void ProcessMouseInput(double xpos, double ypos);

		// boilerplate
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void CreateRenderPass();
		void CreateColorResources();
		void CreateDepthResources();
		bool HasStencilComponent(VkFormat format);
		
		// validation layers
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
		void SetupDebugMessenger();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		// swapchain
		void CreateSwapChain();
		void CreateSurface();
		void CreateImageViews();
		vu::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);
		void CreateFrameBuffers();
		void CleanupSwapChain();
		void RecreateSwapChain();
		
		// memory allocation
		void CreateMemoryAllocator();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties); // now uses vma

		// command buffers
		void CreateCommandPool();
		void CreateCommandBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// shaders
		void CreateShaderModules();
		void destroyShaderModules();

		// image resouses (should be moved)
		void CreateTextureImages();
		void CreateTextureImageViews();
		void CreateTextureImage(const std::string &imagePath, uint32_t &mipLevels, VkImage &image, VmaAllocation &textureImageAllocation, VmaAllocationInfo &textureImageAllocationInfo);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		// samplers
		void CreateTextureSampler();
		
		// descriptors
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateUniformBuffers();
		void CreateDescriptorSets();
		
		// synchronization
		void CreateSyncObjects();

		// scene updated (should be moved)
		void UpdateTime();
		void UpdateTransforms();
		void SetGlobalUniformBuffers(uint32_t currentImage);
		void SetGlobalPushConstants(VkCommandBuffer commandBuffer);


		GLFWwindow *m_window;
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkQueue transferQueue = VK_NULL_HANDLE;

		VmaAllocator allocator;

		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkRenderPass renderPass;

		VkCommandPool graphicsCommandPool;
		VkCommandPool transferCommandPool;
		std::vector<VkCommandBuffer> graphicsCommandBuffers;
		std::vector<VkCommandBuffer> transferCommandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		VkDescriptorSetLayout          descriptorSetLayoutGlobal;
		VkDescriptorSetLayout          descriptorSetLayoutLocal;
		VkPipelineLayout               pipelineLayout;
		VkPipeline                     graphicsPipeline;
		VkDescriptorPool               descriptorPool;
		std::vector<VkBuffer>          uniformBuffers;
		std::vector<VmaAllocation>     uniformAllocations;
		std::vector<VmaAllocationInfo> uniformAllocationInfos;
		std::vector<VkDescriptorSet>   descriptorSetsGlobal;
		std::vector<VkDescriptorSet>   descriptorSetsMat1;
		std::vector<VkDescriptorSet>   descriptorSetsMat2;

		vu::Mesh mesh1;
		vu::Mesh mesh2;

		vu::Material material1;
		vu::Material material2;

		vu::Transform transform1;
		vu::Transform transform2;

		VkImage textureImage1;
		VmaAllocation textureImageAllocation1;
		VmaAllocationInfo textureImageAllocationInfo1;
		VkImageView textureImageView1;
		uint32_t mipLevels1;

		VkImage textureImage2;
		VmaAllocation textureImageAllocation2;
		VmaAllocationInfo textureImageAllocationInfo2;
		VkImageView textureImageView2;
		uint32_t mipLevels2;

		VkSampler textureSampler;

		VkImage depthImage;
		VmaAllocation depthImageAllocation;
		VmaAllocationInfo depthImageAllocationInfo;
		VkImageView depthImageView;

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkImage colorImage;
		VmaAllocation colorImageAllocation;
		VmaAllocationInfo colorImageAllocationInfo;
		VkImageView colorImageView;

		uint32_t currentFrame = 0;
		float lastFrameTime = 0.0f;
		float currentFrameTime = 0.0f;
		float deltaTime = 0.0f;

		float lastX = WIDTH / 2.0f;
		float lastY = HEIGHT / 2.0f;
		float sensitivity = 0.1f;

		vu::Transform camTransform;
		const float camSpeed = 2.0f;

		bool framebufferResized = false;
	};
}
