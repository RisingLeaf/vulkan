#pragma once

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>



class Window {
public:
	Window(int width, int height, std::string name);
	~Window();


	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

	void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

	GLFWwindow *GetWindow() { return window; }
	bool ShouldClose() { return glfwWindowShouldClose(window); }
	VkExtent2D GetExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};}
	bool WasWindowResized() { return framebufferResized; }
	void ResetWindowResizedFlag() { framebufferResized = false; }

private:
	void InitWindow();

	static void FrameBufferResizedCallback(GLFWwindow * window, int width, int height);

	int width, height;
	bool framebufferResized = false;

	std::string windowName;
	GLFWwindow *window;

};