#pragma once

#include <stb/stb_image.h>

#include "vu.h"


namespace vu {

	class Image {
	public:
		// from path
		Image(
			const vu::RendererInfo &renderInfo,
			const std::string &path,
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			bool generateMipMaps = true,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
		) : m_imagePath(path), m_mipMapsGenerated(generateMipMaps), m_imageFormat(format), m_aspectFlags(aspectFlags) {
			CreateTextureImage(renderInfo);
			CreateImageView(renderInfo, m_image, m_imageFormat, m_aspectFlags, m_mipLevels, m_imageView);
		};

		void Destroy(const vu::RendererInfo &rendererInfo) {
			vkDestroyImageView(rendererInfo.device, m_imageView, nullptr);
			vmaDestroyImage(rendererInfo.allocator, m_image, m_ImageAllocation);
		};

		static void CreateImageView(const vu::RendererInfo &rendererInfo, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView &imageView);
		static void CopyBufferToImage(const vu::RendererInfo &rendererInfo, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		static void TransitionImageLayout(const vu::RendererInfo &rendererInfo, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		static bool HasStencilComponent(VkFormat format);

		// getters
		VkImage GetImage() { return m_image; }
		VkImageView GetImageView() { return m_imageView; }

	private:
		void CreateTextureImage(const vu::RendererInfo &rendererInfo);
		void GenerateMipmaps(const vu::RendererInfo &rendererInfo, int32_t texWidth, int32_t texHeight);

		// parameters
		std::string        m_imagePath;
		bool               m_mipMapsGenerated;
		VkFormat           m_imageFormat;
		VkImageAspectFlags m_aspectFlags;

		// creating these
		VkImage           m_image;
		VkImageView       m_imageView;
		VmaAllocation     m_ImageAllocation;
		VmaAllocationInfo m_ImageAllocationInfo;
		uint32_t          m_mipLevels;
	};

}