#include "VulkanGraphicsPipeline.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkDevice logicalDevice) {
	this->_logicalDevice = logicalDevice;

	std::vector<char> vertexShaderCode = readFile("D:/GitRepos/QEngine_Vulkan/Shaders/vert.spv");
	std::vector<char> fragmentShaderCode = readFile("D:/GitRepos/QEngine_Vulkan/Shaders/frag.spv");

	VkShaderModule vertexShaderModule = this->_createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = this->_createShaderModule(fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.module = vertexShaderModule;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.module = fragmentShaderModule;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, vertexShaderCreateInfo};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};

	vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() { }

VkShaderModule VulkanGraphicsPipeline::_createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(this->_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a shader module!..");
	}

	return shaderModule;
}