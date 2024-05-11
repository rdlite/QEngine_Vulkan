#include "VulkanGraphicsCommandPool.h"

VulkanGraphicsCommandPool::VulkanGraphicsCommandPool(VkDevice logicalDevice, QueueFamilyIndicies queueFamilyIndices) {
	this->_logicalDevice = logicalDevice;

	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	VkResult result = vkCreateCommandPool(this->_logicalDevice, &poolCreateInfo, nullptr, &this->_commandPool);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a graphics command pool!..");
	}
}

VulkanGraphicsCommandPool::~VulkanGraphicsCommandPool() {
	vkDestroyCommandPool(this->_logicalDevice, this->_commandPool, nullptr);
}

VkCommandPool VulkanGraphicsCommandPool::getCommandPool() {
	return this->_commandPool;
}
