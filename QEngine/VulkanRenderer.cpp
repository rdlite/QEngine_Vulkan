#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer(GLFWwindow* newWindow) : _window{newWindow} {
	try {
		this->_createInstance();
		this->_createSurface();
		this->_setupDebugMessenger();
		this->_getPhysicalDevice();
		this->_createLogicalDevice();
	}
	catch (const std::runtime_error& e) {
		std::string strErr = e.what();
		Debug::print("Renderer initialize error: " + strErr);
		this->_initResult = EXIT_FAILURE;
		return;
	}

	this->_initResult = EXIT_SUCCESS;
}

VulkanRenderer::~VulkanRenderer() {
	if (this->_enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
	vkDestroyDevice(this->_mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(this->_instance, nullptr);
}

int VulkanRenderer::getInitResult() {
	return this->_initResult;
}

void VulkanRenderer::_getPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		ThrowErr::runtime("Cannot find any gpu's that support Vulkan Instance!..");
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
	if (this->_enableValidationLayers && !_checkValidationLayerSupport()) {
		ThrowErr::runtime("Validation layers requested, but not available!..");
	}

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

	std::vector<const char*> extensions = this->_getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (!this->_checkInstanceExtensionsSupport(&extensions)) {
		ThrowErr::runtime("vkInstance does not support required extensions!..");
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (this->_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		this->_populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a Vulkan Instance!..");
	}
}

void VulkanRenderer::_setupDebugMessenger() {
	if (!this->_enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	this->_populateDebugMessengerCreateInfo(createInfo);

	VkResult result = CreateDebugUtilsMessengerEXT(this->_instance, &createInfo, nullptr, &this->_debugMessenger);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to setup debug messenger!..");
	}
}

void VulkanRenderer::_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void VulkanRenderer::_createLogicalDevice() {
	QueueFamilyIndicies indices = this->_getQueueFamilies(this->_mainDevice.physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	for (int queueFamilyIndex : queueFamilyIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(this->_mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->_mainDevice.logicalDevice);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a logical device!..");
	}

	vkGetDeviceQueue(this->_mainDevice.logicalDevice, indices.graphicsFamily, 0, &this->_graphicsQueue);
	vkGetDeviceQueue(this->_mainDevice.logicalDevice, indices.presentationFamily, 0, &this->_presentationQueue);
}

void VulkanRenderer::_createSurface() {
	VkResult result = glfwCreateWindowSurface(this->_instance, this->_window, nullptr, &this->_surface);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a surface!..");
	}
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

bool VulkanRenderer::_checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
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

		VkBool32 presentationSupport = false;

		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->_surface, &presentationSupport);
		if (queueFamily.queueCount > 0 && presentationSupport) {
			indicies.presentationFamily = i;
		}

		if (indicies.isValid()) {
			break;
		}

		i++;
	}

	return indicies;
}

std::vector<const char*> VulkanRenderer::_getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (this->_enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}