#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer(GLFWwindow* newWindow) : _window{newWindow} {
	try {
		this->_createInstance();
		this->_getPhysicalDevice();
		this->_createLogicalDevice();
	}
	catch (const std::runtime_error& e) {
		printf("Renderer initialize error: %s\n", e.what());
		this->_initResult = EXIT_FAILURE;
		return;
	}

	this->_initResult = EXIT_SUCCESS;
}

VulkanRenderer::~VulkanRenderer() {
	this->cleanup();
}

int VulkanRenderer::getInitResult() {
	return this->_initResult;
}

void VulkanRenderer::cleanup() {
	vkDestroyDevice(this->_mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(this->_instance, nullptr);
}

void VulkanRenderer::_getPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("Cannot find any gpu's that support Vulkan Instance!..");
	}

	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, deviceList.data());

	for (VkPhysicalDevice& device : deviceList) {
		if (this->_checkDeviceSuitable(device)) {
			this->_mainDevice.physicalDevice = device;
			break;
		}
	}
}

void VulkanRenderer::_createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "QEngine Vulkan app";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "QEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	std::vector<const char*> instanceExtensions = std::vector<const char*>();
	uint32_t glfwExtensionCount = 0;

	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (size_t i = 0; i < glfwExtensionCount; i++) {
		instanceExtensions.push_back(*(glfwExtensions + i));
	}

	if (!this->_checkInstanceExtensionsSupport(&instanceExtensions)) {
		throw std::runtime_error("vkInstance does not support required extensions!..");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vulkan Instance!..");
	}
}

void VulkanRenderer::_createLogicalDevice() {
	QueueFamilyIndicies indices = this->_getQueueFamilies(this->_mainDevice.physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount = 1;
	float priority = 1.0f;
	queueCreateInfo.pQueuePriorities = &priority;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(this->_mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->_mainDevice.logicalDevice);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a logical device!..");
	}

	vkGetDeviceQueue(this->_mainDevice.logicalDevice, indices.graphicsFamily, 0, &this->_graphicsQueue);
}

bool VulkanRenderer::_checkInstanceExtensionsSupport(std::vector<const char*>* checkExtensions) {
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

	for (const char* &checkExtension: *checkExtensions) {
		bool hasExtension = false;

		for (VkExtensionProperties &extension : extensions) {
			if (strcmp(checkExtension, extension.extensionName)) {
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension) {
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::_checkDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndicies indices = this->_getQueueFamilies(device);

	return indices.isValid();
}

QueueFamilyIndicies VulkanRenderer::_getQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndicies indicies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	int i = 0;
	for (VkQueueFamilyProperties& queueFamily : queueFamilyList) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indicies.graphicsFamily = i;
		}

		if (indicies.isValid()) {
			break;
		}

		i++;
	}

	return indicies;
}