#include "app.h"

#include "SDL_events.h"
#include "source/es_vulkan.h"
#include "source/logger.h"
#include "source/vulkan_model.h"
#include "source/vulkan_pipeline.h"
#include "source/vulkan_swapchain.h"

#include <GLFW/glfw3.h>
#include <array>
#include <bits/types/time_t.h>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan_core.h>
#include<unistd.h>
#include <chrono>
#include <SOIL/SOIL.h>

namespace Triangle {
	std::vector<float> vertices = {
		// pos       // color
		 0.0f, -0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f,
	};

	std::vector<AttributeSize> attributeDescriptors = {
		AttributeSize::VECTOR_TWO,
		AttributeSize::VECTOR_FOUR,
	};
	
	std::vector<AttributeSize> pushConstantVertexDescriptors = {
		AttributeSize::VECTOR_TWO,
		AttributeSize::VECTOR_THREE,
	};
}



App::App(std::string name)
{
	LoadModel();
	CreatePipelineLayout();
	RecreateSwapChain();
	CreateCommandBuffers();
}


App::~App()
{
	vkDestroyPipelineLayout(device.Device(), pipelineLayout, nullptr);
}



void App::Run()
{
	while(!window.ShouldClose())
	{
		//auto start = std::chrono::high_resolution_clock::now();
		glfwPollEvents();

		DrawFrame();

		/*
		static const double microsecond = 1000000;
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		Logger::Status("Elapsed time is: " + std::to_string(1. / (static_cast<double>(duration.count()) / microsecond)));
		*/
	}

	vkDeviceWaitIdle(device.Device());
}



void App::CreatePipelineLayout()
{
	uint32_t vertexPushConstantsSize = 0;
	for(const auto &pushConstantVertexDescriptor : Triangle::pushConstantVertexDescriptors)
		vertexPushConstantsSize += EsToVulkan::FORMAT_MAP_SIZE.at(pushConstantVertexDescriptor);

	Logger::Status(std::to_string(vertexPushConstantsSize * sizeof(float)));

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = vertexPushConstantsSize * sizeof(float);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if(vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipline layout");
}



void App::CreatePipeline()
{
	assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
	VulkanPipelineConfigInfo pipelineConfig{};
  	VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = swapChain->GetRenderPass();
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<VulkanPipeline>(device,
		"../../resources/shaders/shader.vert.spv", "../../resources/shaders/shader.frag.spv", pipelineConfig, Triangle::attributeDescriptors);
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

	CreatePipeline();
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

	pipeline->Bind(commandBuffers[imageIndex]);
	model->Bind(commandBuffers[imageIndex]);

	for(int j = 0; j < 4; j++)
	{
		std::vector<float> pushConstantData = {
			0.0f, -0.4f + j * 0.25f, // offset
			0.0f, 0.0f, 0.2f + 0.5f * j, // color
		};

		vkCmdPushConstants(
			commandBuffers[imageIndex],
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			5 * sizeof(float),
			pushConstantData.data());
		
		model->Draw(commandBuffers[imageIndex]);
	}

	vkCmdEndRenderPass(commandBuffers[imageIndex]);

	if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");
}



void App::LoadModel()
{
	model = std::make_unique<VulkanModel>(device, Triangle::vertices, Triangle::attributeDescriptors);
}



void App::LoadTexture(const char* filename, VkBuffer &imageBuffer, VkDeviceMemory &imageBufferMemory)
{
	int width, height;
	unsigned char *pixels = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize imageSize = width * height * 4;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
}
