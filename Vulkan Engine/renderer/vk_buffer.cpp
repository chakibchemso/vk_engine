/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "vk_buffer.hpp"

#include <cassert>
#include <cstring>

using vk_engine::vk_buffer;

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instance_size The size of an instance
 * @param min_offset_alignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
VkDeviceSize vk_buffer::get_alignment(const VkDeviceSize instance_size, const VkDeviceSize min_offset_alignment)
{
	if (min_offset_alignment > 0)
	{
		return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
	}
	return instance_size;
}

vk_buffer::vk_buffer(
	vk_device& device,
	const VkDeviceSize instance_size,
	const uint32_t instance_count,
	const VkBufferUsageFlags usage_flags,
	const VkMemoryPropertyFlags memory_property_flags,
	const VkDeviceSize min_offset_alignment)
	: device{device},
	  instance_count{instance_count},
	  instance_size{instance_size},
	  usage_flags{usage_flags},
	  memory_property_flags{memory_property_flags}
{
	alignment_size = get_alignment(instance_size, min_offset_alignment);
	buffer_size = alignment_size * instance_count;
	device.create_buffer(buffer_size, usage_flags, memory_property_flags, buffer, memory);
}

vk_buffer::~vk_buffer()
{
	unmap();
	vkDestroyBuffer(device.get_device(), buffer, nullptr);
	vkFreeMemory(device.get_device(), memory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult vk_buffer::map(const VkDeviceSize size, const VkDeviceSize offset)
{
	assert(buffer && memory && "Called map on buffer before create");
	return vkMapMemory(device.get_device(), memory, offset, size, 0, &mapped);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void vk_buffer::unmap()
{
	if (mapped)
	{
		vkUnmapMemory(device.get_device(), memory);
		mapped = nullptr;
	}
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void vk_buffer::write_to_buffer(const void* data, const VkDeviceSize size, const VkDeviceSize offset) const
{
	assert(mapped && "Cannot copy to unmapped buffer");

	if (size == VK_WHOLE_SIZE)
	{
		memcpy(mapped, data, buffer_size);
	}
	else
	{
		auto mem_offset = static_cast<char*>(mapped);
		mem_offset += offset;
		memcpy(mem_offset, data, size);
	}
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult vk_buffer::flush(const VkDeviceSize size, const VkDeviceSize offset) const
{
	VkMappedMemoryRange mapped_range = {};
	mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped_range.memory = memory;
	mapped_range.offset = offset;
	mapped_range.size = size;
	return vkFlushMappedMemoryRanges(device.get_device(), 1, &mapped_range);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult vk_buffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset) const
{
	VkMappedMemoryRange mapped_range = {};
	mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped_range.memory = memory;
	mapped_range.offset = offset;
	mapped_range.size = size;
	return vkInvalidateMappedMemoryRanges(device.get_device(), 1, &mapped_range);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo vk_buffer::descriptor_info(const VkDeviceSize size, const VkDeviceSize offset) const
{
	return VkDescriptorBufferInfo{
		buffer,
		offset,
		size,
	};
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void vk_buffer::write_to_index(const void* data, const int index) const
{
	write_to_buffer(data, instance_size, index * alignment_size);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult vk_buffer::flush_index(const int index) const
{
	return flush(alignment_size, index * alignment_size);
}

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo vk_buffer::descriptor_info_for_index(const int index) const
{
	return descriptor_info(alignment_size, index * alignment_size);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
VkResult vk_buffer::invalidate_index(const int index) const
{
	return invalidate(alignment_size, index * alignment_size);
}
