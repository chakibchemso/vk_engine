#include "vk_descriptors.hpp"

#include <cassert>
#include <stdexcept>

using vk_engine::vk_descriptor_set_layout;
using vk_engine::vk_descriptor_pool;
using vk_engine::vk_descriptor_writer;

// *************** Descriptor Set Layout Builder *********************

vk_descriptor_set_layout::builder& vk_descriptor_set_layout::builder::add_binding(
	const uint32_t binding,
	const VkDescriptorType descriptor_type,
	const VkShaderStageFlags stage_flags,
	const uint32_t count)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layout_binding{};
	layout_binding.binding = binding;
	layout_binding.descriptorType = descriptor_type;
	layout_binding.descriptorCount = count;
	layout_binding.stageFlags = stage_flags;
	bindings[binding] = layout_binding;
	return *this;
}

std::unique_ptr<vk_descriptor_set_layout> vk_descriptor_set_layout::builder::build() const
{
	return std::make_unique<vk_descriptor_set_layout>(device, bindings);
}

// *************** Descriptor Set Layout *********************

vk_descriptor_set_layout::vk_descriptor_set_layout(
	vk_device& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings)
	: device{device}, bindings{bindings}
{
	std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
	set_layout_bindings.reserve(bindings.size());
	for (auto [fst, snd] : bindings)
		set_layout_bindings.push_back(snd);

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
	descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
	descriptor_set_layout_info.pBindings = set_layout_bindings.data();

	if (vkCreateDescriptorSetLayout(device.get_device(), &descriptor_set_layout_info, nullptr,
	                                &descriptor_set_layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

vk_descriptor_set_layout::~vk_descriptor_set_layout()
{
	vkDestroyDescriptorSetLayout(device.get_device(), descriptor_set_layout, nullptr);
}

// *************** Descriptor Pool Builder *********************

vk_descriptor_pool::builder& vk_descriptor_pool::builder::add_pool_size(
	const VkDescriptorType descriptor_type, const uint32_t count)
{
	pool_sizes.push_back({descriptor_type, count});
	return *this;
}

vk_descriptor_pool::builder& vk_descriptor_pool::builder::set_pool_flags(const VkDescriptorPoolCreateFlags flags)
{
	pool_flags = flags;
	return *this;
}

vk_descriptor_pool::builder& vk_descriptor_pool::builder::set_max_sets(const uint32_t count)
{
	max_sets = count;
	return *this;
}

std::unique_ptr<vk_descriptor_pool> vk_descriptor_pool::builder::build() const
{
	return std::make_unique<vk_descriptor_pool>(device, max_sets, pool_flags, pool_sizes);
}

// *************** Descriptor Pool *********************

vk_descriptor_pool::vk_descriptor_pool(
	vk_device& device,
	const uint32_t max_sets,
	const VkDescriptorPoolCreateFlags pool_flags,
	const std::vector<VkDescriptorPoolSize>& pool_sizes)
	: device{device}
{
	VkDescriptorPoolCreateInfo descriptor_pool_info{};
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	descriptor_pool_info.pPoolSizes = pool_sizes.data();
	descriptor_pool_info.maxSets = max_sets;
	descriptor_pool_info.flags = pool_flags;

	if (vkCreateDescriptorPool(device.get_device(), &descriptor_pool_info, nullptr, &descriptor_pool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}

vk_descriptor_pool::~vk_descriptor_pool()
{
	vkDestroyDescriptorPool(device.get_device(), descriptor_pool, nullptr);
}

bool vk_descriptor_pool::allocate_descriptor(
	const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.pSetLayouts = &descriptor_set_layout;
	alloc_info.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up. But this is beyond our current scope
	if (vkAllocateDescriptorSets(device.get_device(), &alloc_info, &descriptor) != VK_SUCCESS)
		return false;

	return true;
}

void vk_descriptor_pool::free_descriptors(const std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(
		device.get_device(),
		descriptor_pool,
		static_cast<uint32_t>(descriptors.size()),
		descriptors.data());
}

void vk_descriptor_pool::reset_pool() const
{
	vkResetDescriptorPool(device.get_device(), descriptor_pool, 0);
}

// *************** Descriptor Writer *********************

vk_descriptor_writer::vk_descriptor_writer(vk_descriptor_set_layout& set_layout, vk_descriptor_pool& pool)
	: set_layout{set_layout}, pool{pool}
{
}

vk_descriptor_writer& vk_descriptor_writer::write_buffer(
	const uint32_t binding, const VkDescriptorBufferInfo* buffer_info)
{
	assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	const auto& binding_description = set_layout.bindings[binding];

	assert(
		binding_description.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = binding_description.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = buffer_info;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

vk_descriptor_writer& vk_descriptor_writer::write_image(
	const uint32_t binding, const VkDescriptorImageInfo* image_info)
{
	assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	const auto& binding_description = set_layout.bindings[binding];

	assert(
		binding_description.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = binding_description.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = image_info;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

bool vk_descriptor_writer::build(VkDescriptorSet& set)
{
	if (const bool success = pool.allocate_descriptor(set_layout.get_descriptor_set_layout(), set); !success)
		return false;

	overwrite(set);
	return true;
}

void vk_descriptor_writer::overwrite(const VkDescriptorSet& set)
{
	for (auto& write : writes)
		write.dstSet = set;

	vkUpdateDescriptorSets(pool.device.get_device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
