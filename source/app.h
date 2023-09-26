#pragma once

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
	int width = 800, height = 600;

	App(std::string name);
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

	Window window{width, height, "Vulkan Tests"};
	VulkanDevice device{window};
	std::unique_ptr<VulkanSwapChain> swapChain;
	std::unique_ptr<VulkanPipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<VulkanModel> model;

	bool running = true;
};
