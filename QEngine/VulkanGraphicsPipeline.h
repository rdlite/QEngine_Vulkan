#pragma once
#include "VulkanRenderer.h"

class VulkanGraphicsPipeline {
public:
	VulkanGraphicsPipeline(VkDevice logicalDevice);
	~VulkanGraphicsPipeline();
private:
	VkDevice _logicalDevice;
	VkShaderModule _createShaderModule(const std::vector<char> &code);
};