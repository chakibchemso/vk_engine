#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include "../renderer/vk_device.hpp"

namespace vk_engine
{
	class vk_model
	{
	public:
		struct vertex
		{
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
		};

		vk_model(vk_device& device, const std::vector<vertex>& vertices);
		~vk_model();

		vk_model(const vk_model&) = delete;
		vk_model& operator=(const vk_model&) = delete;

		void bind(VkCommandBuffer command_buffer) const;
		void draw(VkCommandBuffer command_buffer) const;

	private:
		void create_vertex_buffers(const std::vector<vertex>& vertices);

		vk_device& device;
		VkBuffer vertex_buffer{};
		VkDeviceMemory vertex_buffer_memory{};
		uint32_t vertex_count{};
	};
}
