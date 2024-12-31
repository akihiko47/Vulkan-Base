#pragma once

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

#define VK_LOD_CLAMP_NONE 15.0f  // max mipmap level for sampler
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <stb/stb_image.h>        // move to image class
#include <VMA/vk_mem_alloc.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "mesh.h"
#include "transform.h"
#include "image.h"
#include "vu.h"


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
		VkInstance       GetInstance()             const { return m_instance; }
		VkPhysicalDevice GetPhysicalDevice()       const { return m_physicalDevice; }
		VkDevice         GetDevice()               const { return m_device; }
		VkFormat         GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
		VkExtent2D       GetSwapChainExtent()      const { return m_swapChainExtent; }
		VkSurfaceKHR     GetSurface()              const { return m_surface; }
		VkRenderPass     GetRenderPass()           const { return m_renderPass; }
		VmaAllocator     GetAllocator()            const { return m_allocator;}
		VkCommandPool    GetGraphicsCommandPool()  const { return m_graphicsCommandPool; }
		VkCommandPool    GetTransferCommandPool()  const { return m_transferCommandPool; }
		VkQueue          GetGraphicsQueue()        const { return m_graphicsQueue; }
		VkQueue			 GetPresentQueue()         const { return m_presentQueue; }
		VkQueue          GetTransferQueue()        const { return m_transferQueue; }

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
		RendererInfo CreateRendererInfo();
		
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

		// materials ubo creation (should be moved)
		void CreateMaterialslBuffers();
		void SetMaterialsBuffers();

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


		VkInstance               m_instance;
		VkPhysicalDevice         m_physicalDevice;
		VkDevice                 m_device;
		VkSwapchainKHR           m_swapChain;
		VkFormat                 m_swapChainImageFormat;
		VkExtent2D               m_swapChainExtent;
		VkSurfaceKHR             m_surface;
		VkRenderPass             m_renderPass;
		VmaAllocator             m_allocator;
		VkDebugUtilsMessengerEXT m_debugMessenger;

		RendererInfo *m_rendererInfo;

		GLFWwindow *m_window;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkQueue m_transferQueue;
		
		std::vector<VkImage>       swapChainImages;
		std::vector<VkImageView>   swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkCommandPool m_graphicsCommandPool;
		VkCommandPool m_transferCommandPool;
		std::vector<VkCommandBuffer> graphicsCommandBuffers;
		std::vector<VkCommandBuffer> transferCommandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence>     inFlightFences;

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

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		vu::Mesh *mesh1;
		vu::Mesh *mesh2;

		vu::Transform transform1;
		vu::Transform transform2;

		vu::Image *image1;
		vu::Image *image2;

		std::vector<VkBuffer>          uniformBuffersMat1;
		std::vector<VmaAllocation>     uniformAllocationsMat1;
		std::vector<VmaAllocationInfo> uniformAllocationInfosMat1;

		std::vector<VkBuffer>          uniformBuffersMat2;
		std::vector<VmaAllocation>     uniformAllocationsMat2;
		std::vector<VmaAllocationInfo> uniformAllocationInfosMat2;

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
