#pragma once
#include "QEngine.h"
#include "VulkanValidation.h"
#include "VulkanUtilities.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFrameBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphicsCommandPool.h"

class VulkanRenderer {
public:
	VulkanRenderer(GLFWwindow* newWindow);
	~VulkanRenderer();
	void draw();
	int getInitResult();

private:
	GLFWwindow* _window = nullptr;
	VkInstance _instance = nullptr;
	VkQueue _graphicsQueue = nullptr;
	VkQueue _presentationQueue = nullptr;
	VkSurfaceKHR _surface = nullptr;
	VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
	VkSwapchainKHR _swapchain = nullptr;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	VulkanGraphicsPipeline* _graphicsPipeline = nullptr;
	VulkanFrameBuffer* _framebuffer = nullptr;
	VulkanGraphicsCommandPool* _graphicsCommandPool = nullptr;
	VulkanCommandBuffer* _commandBuffer = nullptr;

	std::vector<SwapchainImage> _swapchainImages;

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

	VkSemaphore _imageAvailable;
	VkSemaphore _renderFinished;

	void _getPhysicalDevice();
	void _createInstance();
	void _setupDebugMessenger();
	void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void _createLogicalDevice();
	void _createSurface();
	void _createSwapchain();
	void _createGraphicsPipeline();
	void _createFramebuffers();
	void _createGraphicsCommandPool();
	void _createCommandBuffer();
	void _createSynchronization();

	void _recordCommands();

	bool _checkInstanceExtensionsSupport(std::vector<const char*>* checkExtensions);
	bool _checkDeviceSuitable(VkPhysicalDevice device);
	bool _checkValidationLayerSupport();
	bool _checkDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndicies _getQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> _getRequiredExtensions();
	SwapchainDetails _getSwapchainDetails(VkPhysicalDevice device);
	VkImageView _createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	
	VkSurfaceFormatKHR _chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
	VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR> &presentationMods);
	VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
};