#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "QEngine.h"
#include "Utilities.h"
#include "VulkanValidation.h"

class VulkanRenderer {
public:
	VulkanRenderer(GLFWwindow* newWindow);
	~VulkanRenderer();
	int getInitResult();

private:
	GLFWwindow* _window = nullptr;
	VkInstance _instance = nullptr;
	VkQueue _graphicsQueue = nullptr;
	VkQueue _presentationQueue = nullptr;
	VkSurfaceKHR _surface = nullptr;
	VkDebugUtilsMessengerEXT _debugMessenger = nullptr;

#ifdef NDEBUG
	const bool _enableValidationLayers = false;
#else
	const bool _enableValidationLayers = true;
#endif

	int _initResult = 0;

	struct {
		VkPhysicalDevice physicalDevice = nullptr;
		VkDevice logicalDevice = nullptr;
	} _mainDevice;

	void _getPhysicalDevice();
	void _createInstance();
	void _setupDebugMessenger();
	void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void _createLogicalDevice();
	void _createSurface();
	bool _checkInstanceExtensionsSupport(std::vector<const char*>* checkExtensions);
	bool _checkDeviceSuitable(VkPhysicalDevice device);
	bool _checkValidationLayerSupport();
	QueueFamilyIndicies _getQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> _getRequiredExtensions();
};