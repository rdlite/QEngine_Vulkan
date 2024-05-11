#pragma once
#include "QEngine.h"

class ShaderCompiler {
public:
	static VkShaderModule VkCompileVertShaderGLSL(VkDevice logicalDevice, const char* path);
	static VkShaderModule VkCompileFragShaderGLSL(VkDevice logicalDevice, const char* path);
private:
	static VkShaderModule createModuleGLSL(VkDevice logicalDevice, const char* path, std::string shaderType);
};