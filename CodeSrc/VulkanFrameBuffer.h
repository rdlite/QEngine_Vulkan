#pragma once
#include "QEngine.h"
#include "VulkanUtilities.h"

class VulkanFrameBuffer {
public:
	VulkanFrameBuffer(
		std::vector<SwapchainImage> swapchainImages, VkRenderPass renderPass, VkExtent2D swapchainExtent,
		VkDevice logicalDevice);
	~VulkanFrameBuffer();
	std::vector<VkFramebuffer> getSwapchainFramebuffers();
private:
	std::vector<VkFramebuffer> _swapchainFramebuffers;
	VkDevice _logicalDevice;
};