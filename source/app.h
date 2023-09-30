#pragma once

#include "vulkan_descriptors.h"
#include "vulkan_model.h"
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
	struct VulkanPipelineDescription {
		ShaderInfo shaderInfo;
		std::unique_ptr<VulkanPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		VulkanShaderInfo pipelineShaderInfo;
	};

	struct Object {
		std::vector<float> vertices = {
			// pos       // color
			0.0f, -0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
			0.5f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
		} ;

		std::unique_ptr<VulkanModel> model;
	};

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
	void CreatePipelineLayout(VulkanPipelineDescription &pipelineDescription);
	void CreatePipeline(VulkanPipelineDescription &pipelineDescription);
	void CreateCommandBuffers();
	void FreeCommandBuffers();
	void DrawFrame();
	void RecreateSwapChain();
	void RecordCommandBuffer(int imageIndex);
	
	void LoadModel(const ShaderInfo &shaderInfo);

	Window window;
	VulkanDevice device;
	std::unique_ptr<VulkanSwapChain> swapChain;
	std::vector<VulkanPipelineDescription> pipelineDescriptions;
	std::vector<VkCommandBuffer> commandBuffers;

	Object triangle;
};
