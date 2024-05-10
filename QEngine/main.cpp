#include "QEngine.h"
#include "VulkanRenderer.h"

GLFWwindow* initWindow(std::string wName = "Test window", const int width = 800, const int height = 600) {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	return glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main() {
	GLFWwindow* window = initWindow();
	VulkanRenderer* vkRenderer = new VulkanRenderer(window);

	if (vkRenderer->getInitResult() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	delete vkRenderer;
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}