#include "vk_model.hpp"

namespace vk_engine
{
	std::vector<VkVertexInputBindingDescription> vk_model::vertex::get_binding_descriptions()
	{
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof vertex;
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> vk_model::vertex::get_attribute_descriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);

		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(vertex, position);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(vertex, color);

		return attribute_descriptions;
	}

	vk_model::vk_model(vk_device& device, const std::vector<vertex>& vertices): device(device)
	{
		create_vertex_buffers(vertices);
	}

	vk_model::~vk_model()
	{
		vkDestroyBuffer(device.device(), vertex_buffer, nullptr);
		vkFreeMemory(device.device(), vertex_buffer_memory, nullptr);
	}

	void vk_model::bind(const VkCommandBuffer command_buffer) const
	{
		const VkBuffer buffers[] = {vertex_buffer};
		constexpr VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
	}

	void vk_model::draw(const VkCommandBuffer command_buffer) const
	{
		vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
	}

	void vk_model::create_vertex_buffers(const std::vector<vertex>& vertices)
	{
		vertex_count = static_cast<uint32_t>(vertices.size());
		assert(vertex_count >= 3 && "Vertex count must be at least 3");
		const VkDeviceSize buffer_size = sizeof vertices[0] * vertex_count;
		device.create_buffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertex_buffer,
			vertex_buffer_memory);
		void* data;
		vkMapMemory(device.device(), vertex_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), buffer_size);
		vkUnmapMemory(device.device(), vertex_buffer_memory);
	}
}
