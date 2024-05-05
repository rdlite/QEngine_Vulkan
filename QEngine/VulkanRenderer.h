#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "Utilities.h"

class VulkanRenderer {
public:
	VulkanRenderer(GLFWwindow* newWindow);
	~VulkanRenderer();
	int getInitResult();
	void cleanup();

private:
	GLFWwindow* _window = nullptr;
	VkInstance _instance = nullptr;
	VkQueue _graphicsQueue = nullptr;

	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} _mainDevice;

	int _initResult = 0;

	void _getPhysicalDevice();
	void _createInstance();
	void _createLogicalDevice();
	bool _checkInstanceExtensionsSupport(std::vector<const char*>* checkExtensions);
	bool _checkDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndicies _getQueueFamilies(VkPhysicalDevice device);
};