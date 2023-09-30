#include "vulkan_descriptors.h"

#include <cassert>
#include <stdexcept>



VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
	VkShaderStageFlags stageFlags, uint32_t count)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stageFlags;
	bindings[binding] = layoutBinding;
	return *this;
}



std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::build() const
{
	return std::make_unique<VulkanDescriptorSetLayout>(device, bindings);
}



VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
: device{device}, bindings{bindings}
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for(auto kv : bindings)
		setLayoutBindings.push_back(kv.second);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

	if(vkCreateDescriptorSetLayout(device.Device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}



VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device.Device(), descriptorSetLayout, nullptr);
}



VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
	poolSizes.push_back({descriptorType, count});
	return *this;
}



VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
	poolFlags = flags;
	return *this;
}



VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::setMaxSets(uint32_t count)
{
	maxSets = count;
	return *this;
}



std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const
{
	return std::make_unique<VulkanDescriptorPool>(device, maxSets, poolFlags, poolSizes);
}



VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
	const std::vector<VkDescriptorPoolSize> &poolSizes)
: device{device}
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	if(vkCreateDescriptorPool(device.Device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}



VulkanDescriptorPool::~VulkanDescriptorPool()
{
	vkDestroyDescriptorPool(device.Device(), descriptorPool, nullptr);
}



bool VulkanDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	if(vkAllocateDescriptorSets(device.Device(), &allocInfo, &descriptor) != VK_SUCCESS)
		return false;

	return true;
}



void VulkanDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
{
	vkFreeDescriptorSets(device.Device(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}



void VulkanDescriptorPool::resetPool()
{
	vkResetDescriptorPool(device.Device(), descriptorPool, 0);
}



VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDescriptorSetLayout &setLayout, VulkanDescriptorPool &pool)
: setLayout{setLayout}, pool{pool} {}



VulkanDescriptorWriter &VulkanDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto &bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}



VulkanDescriptorWriter &VulkanDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto &bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}



bool VulkanDescriptorWriter::build(VkDescriptorSet &set)
{
	bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
	if (!success)
		return false;

	overwrite(set);
	return true;
}



void VulkanDescriptorWriter::overwrite(VkDescriptorSet &set)
{
	for (auto &write : writes)
		write.dstSet = set;
	vkUpdateDescriptorSets(pool.device.Device(), writes.size(), writes.data(), 0, nullptr);
}