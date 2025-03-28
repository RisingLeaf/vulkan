#include "app.h"

#include "es_vulkan.h"
#include "logger.h"
#include "source/vulkan_texture.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptors.h"
#include "vulkan_device.h"
#include "vulkan_model.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan_core.h>
#include<unistd.h>
#include <chrono>

namespace {
	int texId;
}



App::App(const std::string &name, uint width, uint height)
: width(width), height(height), window(width, height, name), device(window)
{
	std::vector<std::string> paths = {"../../resources/textures/anti-missile hai.png"};
	texId = LoadTexture(paths, 1);
	CreateTextureDescriptors();



	pipelineDescriptions.emplace_back();
	pipelineDescriptions[0].shaderInfo.attributeLayout = {
		AttributeSize::VECTOR_TWO,
		AttributeSize::VECTOR_TWO,
	};
	pipelineDescriptions[0].shaderInfo.uniformLayout = {
		AttributeSize::VECTOR_TWO,
		AttributeSize::VECTOR_THREE,
	};
	pipelineDescriptions[0].shaderInfo.vertexShaderFilename = "../../resources/shaders/shader.vert.spv";
	pipelineDescriptions[0].shaderInfo.fragmentShaderFilename = "../../resources/shaders/shader.frag.spv";
	pipelineDescriptions[0].pipelineShaderInfo = VulkanPipeline::PrepareShaderInfo(device, pipelineDescriptions[0].shaderInfo,
		VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
	CreatePipelineLayout(pipelineDescriptions[0]);
	triangle.model = std::make_unique<VulkanModel>(device, triangle.vertices, pipelineDescriptions[0].shaderInfo.attributeLayout);



	RecreateSwapChain();
	CreateCommandBuffers();
}


App::~App()
{
	for(auto &pipelineDescription : pipelineDescriptions)
		vkDestroyPipelineLayout(device.Device(), pipelineDescription.pipelineLayout, nullptr);
}



void App::Run()
{	
	while(!window.ShouldClose())
	{
		glfwPollEvents();

		DrawFrame();
	}

	vkDeviceWaitIdle(device.Device());
}



void App::CreateTextureDescriptors()
{
	auto minOffsetAlignment = std::lcm(
		device.properties.limits.minUniformBufferOffsetAlignment,
		device.properties.limits.nonCoherentAtomSize);

	desriptorPool = VulkanDescriptorPool::Builder(device)
		.SetMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
		.Build();
	desriptorSetLayout = VulkanDescriptorSetLayout::Builder(device)
		.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();
	for(int j = 0; j < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; j++)
	{
		descriptorSets.emplace_back();
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = textures[0][texId]->GetImageLayout();
		imageInfo.imageView = textures[0][texId]->GetImageView();
		imageInfo.sampler = textures[0][texId]->GetSampler();
		VulkanDescriptorWriter(*desriptorSetLayout, *desriptorPool)
			.WriteImage(1, &imageInfo)
			.Build(descriptorSets.back());
	}
}



void App::CreatePipelineLayout(VulkanPipelineDescription &pipelineDescription)
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
		pipelineDescription.pipelineShaderInfo.desriptorSetLayout->GetDescriptorSetLayout(),
		desriptorSetLayout->GetDescriptorSetLayout()
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if(vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineDescription.pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipline layout");
}



void App::RecreateSwapChain()
{
	auto extent = window.GetExtent();
	while(extent.width == 0 || extent.height == 0)
	{
		extent = window.GetExtent();
		glfwWaitEvents();
	}		

	vkDeviceWaitIdle(device.Device());
	Logger::Status("Creating SwapChain");
	if(swapChain == nullptr)
		swapChain = std::make_unique<VulkanSwapChain>(device, extent);
	else
	{
		swapChain = std::make_unique<VulkanSwapChain>(device, extent, std::move(swapChain));
		if(swapChain->ImageCount() != commandBuffers.size())
		{
			FreeCommandBuffers();
			CreateCommandBuffers();
		}
	}

	for(auto &pipelineDescription : pipelineDescriptions)
		CreatePipeline(pipelineDescription);
}



void App::CreatePipeline(VulkanPipelineDescription &pipelineDescription)
{
	assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(pipelineDescription.pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
	VulkanPipelineConfigInfo pipelineConfig{};
  	VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = swapChain->GetRenderPass();
	pipelineConfig.pipelineLayout = pipelineDescription.pipelineLayout;
	pipelineDescription.pipeline = std::make_unique<VulkanPipeline>(
		device,
		pipelineDescription.shaderInfo.vertexShaderFilename,
		pipelineDescription.shaderInfo.fragmentShaderFilename,
		pipelineConfig,
		pipelineDescription.shaderInfo.attributeLayout);
}



void App::CreateCommandBuffers()
{
	commandBuffers.resize(swapChain->ImageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = device.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(device.Device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers!");
}



void App::FreeCommandBuffers()
{
	vkFreeCommandBuffers(device.Device(), device.GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
}



void App::DrawFrame()
{
	uint32_t imageIndex;
	auto result = swapChain->AcquireNextImage(&imageIndex);

	if(result == VK_ERROR_OUT_OF_DATE_KHR)
		RecreateSwapChain();
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image");

	RecordCommandBuffer(imageIndex);
	result = swapChain->SubmitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.WasWindowResized())
	{
		RecreateSwapChain();
		window.ResetWindowResizedFlag();
		return;
	}

	if(result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image");
}



void App::RecordCommandBuffer(int imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = swapChain->GetRenderPass();
	renderPassInfo.framebuffer = swapChain->GetFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChain->GetSwapChainExtent().width);
	viewport.height = static_cast<float>(swapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{{0, 0}, swapChain->GetSwapChainExtent()};
	vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

	pipelineDescriptions[0].pipeline->Bind(commandBuffers[imageIndex]);
	triangle.model->Bind(commandBuffers[imageIndex]);

	vkCmdBindDescriptorSets(
			commandBuffers[imageIndex],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineDescriptions[0].pipelineLayout,
			1,
			1,
			&descriptorSets[imageIndex],
			0,
			nullptr);


	std::vector<uint32_t> offsets = pipelineDescriptions[0].pipelineShaderInfo.GetDynamicOffsets(imageIndex, 4);

	for(int j = 0; j < 4; j++)
	{
		std::vector<float> uniformData = {
			0.0f, -0.1f * j, 0.0f, 0.0f, // offset
			0.1f * j,  0.0f, 0.25f * j, // color
		};

		uint32_t bufferIndex = (pipelineDescriptions[0].pipelineShaderInfo.bufferCount * imageIndex) + j;
		pipelineDescriptions[0].pipelineShaderInfo.uniformBuffer->WriteToIndex(uniformData.data(), bufferIndex);
		pipelineDescriptions[0].pipelineShaderInfo.uniformBuffer->FlushIndex(bufferIndex);

		vkCmdBindDescriptorSets(
			commandBuffers[imageIndex],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineDescriptions[0].pipelineLayout,
			0,
			1,
			&pipelineDescriptions[0].pipelineShaderInfo.descriptorSets[bufferIndex],
			1,
			offsets.data());
		
		triangle.model->Draw(commandBuffers[imageIndex]);
	}

	vkCmdEndRenderPass(commandBuffers[imageIndex]);

	if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");
}



int App::LoadTexture(const std::vector<std::string> &filepaths, uint binding)
{
	assert(binding > 0 && "Binding 0 is reserved for the uniform buffer.");
	textures[binding - 1].emplace_back(std::make_unique<VulkanTexture>(device, filepaths));
	return textures[binding - 1].size() - 1;
}
