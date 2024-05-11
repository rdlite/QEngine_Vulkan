#pragma once
#include "QEngine.h"
#include "VulkanUtilities.h"

class VulkanGraphicsCommandPool {
public:
	VulkanGraphicsCommandPool(VkDevice logicalDevice, QueueFamilyIndicies queueFamilyIndices);
	~VulkanGraphicsCommandPool();
	VkCommandPool getCommandPool();
private:
	VkDevice _logicalDevice;
	VkCommandPool _commandPool;
};