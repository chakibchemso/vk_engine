#pragma once

#include "../renderer/vk_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vk_engine
{
	class vk_descriptor_set_layout
	{
	public:
		class builder
		{
		public:
			builder(vk_device& device) : device{device}
			{
			}

			builder& add_binding(
				uint32_t binding,
				VkDescriptorType descriptor_type,
				VkShaderStageFlags stage_flags,
				uint32_t count = 1);
			std::unique_ptr<vk_descriptor_set_layout> build() const;

		private:
			vk_device& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		vk_descriptor_set_layout(vk_device& device,
		                         const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
		~vk_descriptor_set_layout();
		vk_descriptor_set_layout(const vk_descriptor_set_layout&) = delete;
		vk_descriptor_set_layout& operator=(const vk_descriptor_set_layout&) = delete;

		VkDescriptorSetLayout get_descriptor_set_layout() const { return descriptor_set_layout; }

	private:
		vk_device& device;
		VkDescriptorSetLayout descriptor_set_layout{};
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class vk_descriptor_writer;
	};

	class vk_descriptor_pool
	{
	public:
		class builder
		{
		public:
			builder(vk_device& device) : device{device}
			{
			}

			builder& add_pool_size(VkDescriptorType descriptor_type, uint32_t count);
			builder& set_pool_flags(VkDescriptorPoolCreateFlags flags);
			builder& set_max_sets(uint32_t count);
			std::unique_ptr<vk_descriptor_pool> build() const;

		private:
			vk_device& device;
			std::vector<VkDescriptorPoolSize> pool_sizes{};
			uint32_t max_sets = 1000;
			VkDescriptorPoolCreateFlags pool_flags = 0;
		};

		vk_descriptor_pool(
			vk_device& device,
			uint32_t max_sets,
			VkDescriptorPoolCreateFlags pool_flags,
			const std::vector<VkDescriptorPoolSize>& pool_sizes);
		~vk_descriptor_pool();
		vk_descriptor_pool(const vk_descriptor_pool&) = delete;
		vk_descriptor_pool& operator=(const vk_descriptor_pool&) = delete;

		bool allocate_descriptor(
			VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const;

		void free_descriptors(const std::vector<VkDescriptorSet>& descriptors) const;

		void reset_pool() const;

	private:
		vk_device& device;
		VkDescriptorPool descriptor_pool{};

		friend class vk_descriptor_writer;
	};

	class vk_descriptor_writer
	{
	public:
		vk_descriptor_writer(vk_descriptor_set_layout& set_layout, vk_descriptor_pool& pool);

		vk_descriptor_writer& write_buffer(uint32_t binding, const VkDescriptorBufferInfo* buffer_info);
		vk_descriptor_writer& write_image(uint32_t binding, const VkDescriptorImageInfo* image_info);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		vk_descriptor_set_layout& set_layout;
		vk_descriptor_pool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};
}
