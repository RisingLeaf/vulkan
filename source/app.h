#pragma once

#include "source/vulkan_texture.h"
#include "vulkan_descriptors.h"
#include "vulkan_model.h"
#include "window.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"

#include <array>
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
			0.0f, -0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f, 0.5f,  0.0f, 1.0f,
		} ;

		std::unique_ptr<VulkanModel> model;
	};

public:
	uint width, height;
	uint frame;

	App(const std::string &name, uint width, uint height);
	~App();

	App(const App &) = delete;
	App operator=(const App &) = delete;
	App(App &&) = delete;
	App &operator=(App &&) = delete;


	void Run();

private:
	void CreateTextureDescriptors();

	void CreatePipelineLayout(VulkanPipelineDescription &pipelineDescription);
	void RecreateSwapChain();
	void CreatePipeline(VulkanPipelineDescription &pipelineDescription);

	void CreateCommandBuffers();
	void FreeCommandBuffers();

	void DrawFrame();
	void RecordCommandBuffer(int imageIndex);

	
	int LoadTexture(const std::vector<std::string> &filepaths, uint binding);

	Window window;
	VulkanDevice device;
	std::unique_ptr<VulkanSwapChain> swapChain;
	std::vector<VulkanPipelineDescription> pipelineDescriptions;
	std::vector<VkCommandBuffer> commandBuffers;

	Object triangle;

	std::vector<std::unique_ptr<VulkanTexture>> textures[2];
	std::unique_ptr<VulkanDescriptorPool> desriptorPool;
	std::unique_ptr<VulkanDescriptorSetLayout> desriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
};
