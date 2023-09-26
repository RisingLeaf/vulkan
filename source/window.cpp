#include "window.h"
#include "logger.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>



Window::Window(int width, int height, std::string name)
: width(width), height(height), windowName(name)
{
	InitWindow();
}



Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}



void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");
}



void Window::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FrameBufferResizedCallback);
}



void Window::FrameBufferResizedCallback(GLFWwindow * window, int width, int height)
{
	auto windowPointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	windowPointer->framebufferResized = true;
	windowPointer->width = width;
	windowPointer->height = height;
}
