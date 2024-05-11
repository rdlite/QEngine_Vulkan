#include "ShaderCompiler.h"

VkShaderModule ShaderCompiler::VkCompileVertShaderGLSL(VkDevice logicalDevice, const char* path) {
	return createModuleGLSL(logicalDevice, path, "vert");
}

VkShaderModule ShaderCompiler::VkCompileFragShaderGLSL(VkDevice logicalDevice, const char* path) {
	return createModuleGLSL(logicalDevice, path, "frag");
}

VkShaderModule ShaderCompiler::createModuleGLSL(VkDevice logicalDevice, const char* path, std::string shaderType) {
	std::string dirname = QString::getDirname(std::string(path));

	std::string shaderStartFile = path;
	std::string spvFilePath = SPV_GLSL_SHADERS_OUTPUT_PATH + "temp_shader_" + shaderType + ".spv";

	system(("cd " + dirname + " & " + SPIRV_GLSL_SHADER_COMPILER_BATCH_PATH + " " + shaderStartFile + " " + spvFilePath).c_str());

	std::vector<char> code = readFile(spvFilePath.c_str());

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a shader module by " + spvFilePath + " path!..");
	}

	return shaderModule;
}
