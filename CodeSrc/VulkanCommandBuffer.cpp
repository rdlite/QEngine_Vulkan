#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::VulkanCommandBuffer(
	VkDevice logicalDevice, std::vector<SwapchainImage> swapchainBuffers, VulkanGraphicsCommandPool* commandPoolDesc) {
	this->_logicalDevice = logicalDevice;

	this->_commandBuffers.resize(swapchainBuffers.size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = commandPoolDesc->getCommandPool();
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(this->_commandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(this->_logicalDevice, &cbAllocInfo, this->_commandBuffers.data());
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to allocate command buffers!..");
	}
}

VulkanCommandBuffer::~VulkanCommandBuffer(){}

std::vector<VkCommandBuffer> VulkanCommandBuffer::getCommandBuffers() {
	return this->_commandBuffers;
}
