#pragma once

#include "vk_device.hpp"

namespace vk_engine
{
	class vk_buffer
	{
	public:
		vk_buffer(
			vk_device& device,
			VkDeviceSize instance_size,
			uint32_t instance_count,
			VkBufferUsageFlags usage_flags,
			VkMemoryPropertyFlags memory_property_flags,
			VkDeviceSize min_offset_alignment = 1);
		~vk_buffer();

		vk_buffer(const vk_buffer&) = delete;
		vk_buffer& operator=(const vk_buffer&) = delete;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		void write_to_buffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		VkDescriptorBufferInfo descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

		void write_to_index(const void* data, int index) const;
		VkResult flush_index(int index) const;
		VkDescriptorBufferInfo descriptor_info_for_index(int index) const;
		VkResult invalidate_index(int index) const;

		VkBuffer get_buffer() const { return buffer; }
		void* get_mapped_memory() const { return mapped; }
		uint32_t get_instance_count() const { return instance_count; }
		VkDeviceSize get_instance_size() const { return instance_size; }
		VkDeviceSize get_alignment_size() const { return instance_size; }
		VkBufferUsageFlags get_usage_flags() const { return usage_flags; }
		VkMemoryPropertyFlags get_memory_property_flags() const { return memory_property_flags; }
		VkDeviceSize get_buffer_size() const { return buffer_size; }

	private:
		static VkDeviceSize get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

		vk_device& device;
		void* mapped = nullptr;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize buffer_size;
		uint32_t instance_count;
		VkDeviceSize instance_size;
		VkDeviceSize alignment_size;
		VkBufferUsageFlags usage_flags;
		VkMemoryPropertyFlags memory_property_flags;
	};
}
