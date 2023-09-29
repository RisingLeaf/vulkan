#pragma once

#include "vulkan_device.h"

#include <string.h>
#include <vulkan/vulkan_core.h>

class VulkanTexture {
public:
	VulkanTexture(VulkanDevice &device, const std::string &filepath);
	~VulkanTexture();

	VulkanTexture(const VulkanTexture &) = delete;
	VulkanTexture &operator=(const VulkanTexture &) = delete;
	VulkanTexture(VulkanTexture &&) = delete;
	VulkanTexture &operator=(VulkanTexture &&) = delete;

	VkSampler GetSampler() { return sampler; }
	VkImageView GetImageView() { return imageView; }
	VkImageLayout GetImageLayout() { return imageLayout; }
private:
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
	void GenerateMipmaps();

	int width, height, mipLevels;

	VulkanDevice &device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;
	VkFormat imageFormat;
	VkImageLayout imageLayout;
};