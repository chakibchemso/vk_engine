#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "vk_device.hpp"
#include <glm/glm.hpp>

namespace vk_engine
{
	class vk_model
	{
	public:
		struct vertex
		{
			glm::vec2 position;

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
		};
		
		vk_model(vk_device &device, const std::vector<vertex> &vertices);
		~vk_model();

		vk_model(const vk_model&) = delete;
		vk_model& operator=(const vk_model&) = delete;

		void bind(VkCommandBuffer command_buffer) const;
		void draw(VkCommandBuffer command_buffer) const;
		
	private:
		void create_vertex_buffers(const std::vector<vertex> &vertices);
		
		vk_device &device;
		VkBuffer vertex_buffer{};
		VkDeviceMemory vertex_buffer_memory{};
		uint32_t vertex_count{};
	};
}