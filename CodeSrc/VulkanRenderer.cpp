#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer(GLFWwindow* newWindow) : _window{newWindow} {
	try {
		this->_createInstance();
		this->_setupDebugMessenger();
		this->_createSurface();
		this->_getPhysicalDevice();
		this->_createLogicalDevice();
		this->_createSwapchain();
		this->_createGraphicsPipeline();
		this->_createFramebuffers();
		this->_createGraphicsCommandPool();
		this->_createCommandBuffer();
		this->_recordCommands();
		this->_createSynchronization();
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
	vkDeviceWaitIdle(this->_mainDevice.logicalDevice);

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		vkDestroyFence(this->_mainDevice.logicalDevice, this->_drawFences[i], nullptr);
		vkDestroySemaphore(this->_mainDevice.logicalDevice, this->_rendersFinished[i], nullptr);
		vkDestroySemaphore(this->_mainDevice.logicalDevice, this->_imagesAvailable[i], nullptr);
	}
	
	delete this->_commandBuffer;
	delete this->_graphicsCommandPool;
	delete this->_framebuffer;
	delete this->_graphicsPipeline;

	for (auto image : this->_swapchainImages) {
		vkDestroyImageView(this->_mainDevice.logicalDevice, image.imageView, nullptr);
	}
	
	vkDestroySwapchainKHR(this->_mainDevice.logicalDevice, this->_swapchain, nullptr);
	
	if (this->_enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
	vkDestroyDevice(this->_mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(this->_instance, nullptr);
}

void VulkanRenderer::draw() {
	vkWaitForFences(
		this->_mainDevice.logicalDevice,
		1,
		&this->_drawFences[_currentFrame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	vkResetFences(this->_mainDevice.logicalDevice, 1, &this->_drawFences[_currentFrame]);
	
	uint32_t imageIndex;
	vkAcquireNextImageKHR(
		this->_mainDevice.logicalDevice,
		this->_swapchain,
		std::numeric_limits<uint64_t>::max(),
		this->_imagesAvailable[_currentFrame],
		VK_NULL_HANDLE,
		&imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &this->_imagesAvailable[_currentFrame];
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	auto constBuffs = this->_commandBuffer->getCommandBuffers();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &constBuffs[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &this->_rendersFinished[_currentFrame];

	VkResult result = vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, this->_drawFences[_currentFrame]);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to submit command buffer to render queue!..");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &this->_rendersFinished[_currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &this->_swapchain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(this->_presentationQueue, &presentInfo);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to present Image!..");
	}

	this->_currentFrame = (this->_currentFrame + 1) % MAX_FRAME_DRAWS;
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

	int maxDeviceScore = -1;
	for (VkPhysicalDevice& device : deviceList) {
		int score = _rateDeviceSuitability(device);

		if (maxDeviceScore < score && this->_checkDeviceSuitable(device)) {
			maxDeviceScore = score;
			this->_mainDevice.physicalDevice = device;
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
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

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

void VulkanRenderer::_createSwapchain() {
	SwapchainDetails swapchainDetails = this->_getSwapchainDetails(this->_mainDevice.physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = this->_chooseSurfaceFormat(swapchainDetails.formats);
	VkPresentModeKHR presentMode = this->_choosePresentMode(swapchainDetails.presentationModes);
	VkExtent2D resolution = this->_chooseSwapExtent(swapchainDetails.surfaceCapabilities);

	uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;

	if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount) {
		imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = this->_surface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = resolution;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = swapchainDetails.surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.presentMode = presentMode;

	QueueFamilyIndicies indices = this->_getQueueFamilies(this->_mainDevice.physicalDevice);
	if (indices.graphicsFamily != indices.presentationFamily) {
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily,
		};

		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(this->_mainDevice.logicalDevice, &swapchainCreateInfo, nullptr, &this->_swapchain);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create a swapchain!..");
	}

	this->_swapchainImageFormat = surfaceFormat.format;
	this->_swapchainExtent = resolution;

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(this->_mainDevice.logicalDevice, this->_swapchain, &swapchainImageCount, nullptr);
	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(this->_mainDevice.logicalDevice, this->_swapchain, &swapchainImageCount, images.data());

	this->_swapchainImages.clear();
	for (VkImage image : images) {
		SwapchainImage swapchainImage = {};
		swapchainImage.image = image;
		swapchainImage.imageView = this->_createImageView(image, _swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		this->_swapchainImages.push_back(swapchainImage);
	}
}

void VulkanRenderer::_createGraphicsPipeline() {
	this->_graphicsPipeline = new VulkanGraphicsPipeline(
		this->_mainDevice.logicalDevice, this->_swapchainExtent, this->_swapchainImageFormat);
}

void VulkanRenderer::_createFramebuffers() {
	this->_framebuffer = new VulkanFrameBuffer(
		this->_swapchainImages, this->_graphicsPipeline->getRenderPass(), this->_swapchainExtent,
		this->_mainDevice.logicalDevice);
}

void VulkanRenderer::_createCommandBuffer() {
	this->_commandBuffer = new VulkanCommandBuffer(
		this->_mainDevice.logicalDevice, this->_swapchainImages, this->_graphicsCommandPool);
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
	QueueFamilyIndicies indices = this->_getQueueFamilies(device);
	bool extensionsSupported = this->_checkDeviceExtensionSupport(device);

	SwapchainDetails scDetails = this->_getSwapchainDetails(device);
	bool swapchainValid = !scDetails.presentationModes.empty() && !scDetails.formats.empty();

	return indices.isValid() && extensionsSupported && swapchainValid;
}

int VulkanRenderer::_rateDeviceSuitability(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
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

bool VulkanRenderer::_checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

	if (extensionsCount == 0) {
		return false;
	}

	std::vector<VkExtensionProperties> extensions(extensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

	for (const char* deviceExtension : deviceExtensions) {
		bool hasExtension = false;

		for (const auto& extension : extensions) {
			if (strcmp(deviceExtension, extension.extensionName)) {
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

void VulkanRenderer::_recordCommands() {
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = this->_swapchainExtent;
	VkClearValue clearValue[] = {
		{0.0f, 0.0f, 0.0f, 1.0f}
	};
	renderPassBeginInfo.pClearValues = clearValue;
	renderPassBeginInfo.renderPass = this->_graphicsPipeline->getRenderPass();
	renderPassBeginInfo.clearValueCount = 1;

	for (size_t i = 0; i < this->_commandBuffer->getCommandBuffers().size(); i++) {
		VkCommandBuffer cb = this->_commandBuffer->getCommandBuffers()[i];

		renderPassBeginInfo.framebuffer = this->_framebuffer->getSwapchainFramebuffers()[i];

		VkResult result = vkBeginCommandBuffer(cb, &bufferBeginInfo);
		if (result != VK_SUCCESS) {
			ThrowErr::runtime("Failed to start recording a command buffer!..");
		}

		vkCmdBeginRenderPass(cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_graphicsPipeline->getPipeline());
		vkCmdDraw(cb, 3, 1, 0, 0);
		vkCmdEndRenderPass(cb);

		result = vkEndCommandBuffer(cb);
		if (result != VK_SUCCESS) {
			ThrowErr::runtime("Failed to stop recording a command buffer!..");
		}
	}
}

void VulkanRenderer::_createGraphicsCommandPool() {
	this->_graphicsCommandPool = new VulkanGraphicsCommandPool(this->_mainDevice.logicalDevice, this->_getQueueFamilies(this->_mainDevice.physicalDevice));
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

SwapchainDetails VulkanRenderer::_getSwapchainDetails(VkPhysicalDevice device) {
	SwapchainDetails swapchainDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->_surface, &swapchainDetails.surfaceCapabilities);

	uint32_t formatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface, &formatsCount, nullptr);
	if (formatsCount != 0) {
		swapchainDetails.formats.resize(formatsCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface, &formatsCount, swapchainDetails.formats.data());
	}

	uint32_t presentationsCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface, &presentationsCount, nullptr);
	if (presentationsCount != 0) {
		swapchainDetails.presentationModes.resize(presentationsCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface, &presentationsCount, swapchainDetails.presentationModes.data());
	}

	return swapchainDetails;
}

VkImageView VulkanRenderer::_createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult result = vkCreateImageView(this->_mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS) {
		ThrowErr::runtime("Failed to create image view!..");
	}

	return imageView;
}

void VulkanRenderer::_createSynchronization() {
	this->_imagesAvailable.resize(MAX_FRAME_DRAWS);
	this->_rendersFinished.resize(MAX_FRAME_DRAWS);
	this->_drawFences.resize(MAX_FRAME_DRAWS);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		if (vkCreateSemaphore(this->_mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &this->_imagesAvailable[i]) != VK_SUCCESS) {
			ThrowErr::runtime("Failed to create a image available semaphore!..");
		}

		if (vkCreateSemaphore(this->_mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &this->_rendersFinished[i]) != VK_SUCCESS) {
			ThrowErr::runtime("Failed to create a render semaphore!..");
		}

		if (vkCreateFence(this->_mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &this->_drawFences[i]) != VK_SUCCESS) {
			ThrowErr::runtime("Failed to create a draw fence!..");
		}
	}
}

VkSurfaceFormatKHR VulkanRenderer::_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : formats) {
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR VulkanRenderer::_choosePresentMode(const std::vector<VkPresentModeKHR>& presentationMods) {
	for (const auto& presentationMode : presentationMods) {
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentationMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return surfaceCapabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(this->_window, &width, &height);

		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}