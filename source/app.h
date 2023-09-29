#pragma once

#include "source/vulkan_descriptors.h"
#include "source/vulkan_model.h"
#include "window.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"

#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>



class App {
public:
	int width, height;

	App(std::string name, int width, int height);
	~App();

	App(const App &) = delete;
	App operator=(const App &) = delete;
	App(App &&) = delete;
	App &operator=(App &&) = delete;


	void Run();

private:
	void CreatePipelineLayout();
	void CreatePipeline();
	void CreateCommandBuffers();
	void FreeCommandBuffers();
	void DrawFrame();
	void RecreateSwapChain();
	void RecordCommandBuffer(int imageIndex);
	
	void LoadModel();
	void LoadTexture(const char* filename, VkBuffer &imageBuffer, VkDeviceMemory &imageBufferMemory);

	Window window;
	VulkanDevice device;
	std::unique_ptr<VulkanSwapChain> swapChain;
	std::unique_ptr<VulkanPipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<VulkanModel> model;

	struct Triangle {
		std::vector<float> vertices = {
			// pos       // color
			0.0f, -0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
			0.5f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
			-0.5f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
		} ;

		ShaderInfo shaderinfo;
	} triangle;
};
