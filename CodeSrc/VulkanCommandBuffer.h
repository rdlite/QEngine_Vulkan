#pragma once
#include "QEngine.h"
#include "VulkanUtilities.h"
#include "VulkanGraphicsCommandPool.h"

class VulkanCommandBuffer {
public:
	VulkanCommandBuffer(
		VkDevice logicalDevice, std::vector<SwapchainImage> swapchainImages, VulkanGraphicsCommandPool* commandPoolDesc);
	~VulkanCommandBuffer();
	std::vector<VkCommandBuffer> getCommandBuffers();
private:
	VkDevice _logicalDevice;
	std::vector<VkCommandBuffer> _commandBuffers;
};