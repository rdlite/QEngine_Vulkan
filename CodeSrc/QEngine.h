#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <array>
#include <set>

#include "Debug.h"
#include "ThrowErr.h"
#include "ShaderCompiler.h"
#include "QString.h"

const std::string SPIRV_GLSL_SHADER_COMPILER_BATCH_PATH = "C:/Users/rdlit/QEngine/Libs/Executors/SPIR-VGLSLCompiler.bat";
const std::string SPV_GLSL_SHADERS_OUTPUT_PATH = "C:/Users/rdlit/QEngine/Shaders/temp/";

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open()) {
		ThrowErr::runtime("Failed to open a file!..");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);
	file.close();

	return fileBuffer;
}