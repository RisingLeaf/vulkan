#include "vulkan_model.h"
#include "source/es_vulkan.h"
#include "source/logger.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>



VulkanModel::VulkanModel(VulkanDevice &device, const std::vector<float> &vertices, const std::vector<AttributeSize> &attributeDescriptors)
: device(device), attributeDescriptors(attributeDescriptors)
{
	CreateVertexBuffers(vertices);
}



VulkanModel::~VulkanModel()
{
	vkDestroyBuffer(device.Device(), vertexBuffer, nullptr);
	vkFreeMemory(device.Device(), vertexBufferMemory, nullptr);
}



void VulkanModel::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = {vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}



void VulkanModel::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}



void VulkanModel::CreateVertexBuffers(const std::vector<float> &vertices)
{
	uint32_t sizePerVertex = 0;
	for(const auto &attributeDescriptor : attributeDescriptors)
		sizePerVertex += EsToVulkan::FORMAT_MAP_SIZE.at(attributeDescriptor);

	vertexCount = static_cast<uint32_t>(vertices.size() / sizePerVertex);
	assert(vertexCount >= 3 && "vertex count must be at least 3");
	VkDeviceSize bufferSize = sizeof(float) * vertexCount * sizePerVertex;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);
	
	void *data;

	vkMapMemory(device.Device(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device.Device(), stagingBufferMemory);

	device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	device.CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);
	vkDestroyBuffer(device.Device(), stagingBuffer, nullptr);
	vkFreeMemory(device.Device(), stagingBufferMemory, nullptr);
}
