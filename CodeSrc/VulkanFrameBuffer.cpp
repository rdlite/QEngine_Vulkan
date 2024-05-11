#include "VulkanFrameBuffer.h"

VulkanFrameBuffer::VulkanFrameBuffer(
	std::vector<SwapchainImage> swapchainImages, VkRenderPass renderPass, VkExtent2D swapchainExtent,
	VkDevice logicalDevice) {
	this->_logicalDevice = logicalDevice;
	this->_swapchainFramebuffers.resize(swapchainImages.size());

	for (size_t i = 0; i < this->_swapchainFramebuffers.size(); i++) {
		std::array<VkImageView, 1> attachments = {
			swapchainImages[i].imageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapchainExtent.width;
		framebufferCreateInfo.height = swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(this->_logicalDevice, &framebufferCreateInfo, nullptr, &this->_swapchainFramebuffers[i]);
		if (result != VK_SUCCESS) {
			ThrowErr::runtime("Failed to create a framebuffer!..");
		}
	}
}

VulkanFrameBuffer::~VulkanFrameBuffer() {
	for (size_t i = 0; i < this->_swapchainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(this->_logicalDevice, this->_swapchainFramebuffers[i], nullptr);
	}

	this->_swapchainFramebuffers.clear();
}

std::vector<VkFramebuffer> VulkanFrameBuffer::getSwapchainFramebuffers() {
	return this->_swapchainFramebuffers;
}
