#pragma once

#include "vu.h"

class Image {
public:
	Image(const std::string &path, VkFormat format, bool generateMipMaps = true);
	~Image();

	VkImage GetImage();
	VkImage GetImageView();

private:
	VkImage m_image;
	VkImageView m_imageView;
	VmaAllocation m_textureImageAllocation;
	VmaAllocationInfo m_textureImageAllocationInfo;
	uint32_t m_mipLevels;
};