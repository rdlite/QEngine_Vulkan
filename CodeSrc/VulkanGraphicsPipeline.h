#pragma once
#include "QEngine.h"
#include "ShaderCompiler.h"

class VulkanGraphicsPipeline {
public:
	VulkanGraphicsPipeline(VkDevice logicalDevice, VkExtent2D swapchainExtent, VkFormat swapchainImageFormat);
	~VulkanGraphicsPipeline();
	VkRenderPass getRenderPass();
	VkPipeline getPipeline();
private:
	VkPipeline _graphicsPipeline;
	VkDevice _logicalDevice;
	VkPipelineLayout _pipelineLayout;
	VkFormat _swapchainImageFormat;
	VkRenderPass _renderPass;
	
	VkShaderModule _createShaderModule(const std::vector<char>& code);
	void _createRenderPass();
};