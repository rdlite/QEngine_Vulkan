#pragma once
#include "QEngine.h"

class VulkanGraphicsPipeline {
public:
	VulkanGraphicsPipeline(VkDevice logicalDevice, VkExtent2D swapchainExtent, VkFormat swapchainImageFormat);
	~VulkanGraphicsPipeline();
private:
	VkPipeline _graphicsPipeline;
	VkDevice _logicalDevice;
	VkPipelineLayout _pipelineLayout;
	VkFormat _swapchainImageFormat;
	VkRenderPass _renderPass;
	
	VkShaderModule _createShaderModule(const std::vector<char>& code);
	void _createRenderPass();
};