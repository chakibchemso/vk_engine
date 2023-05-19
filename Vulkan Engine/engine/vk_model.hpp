#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <memory>
#include <glm/glm.hpp>
#include "../renderer/vk_buffer.hpp"
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
			glm::vec3 normal;
			glm::vec2 uv;

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

			bool operator==(const vertex& other) const;
		};

		struct builder
		{
			std::vector<vertex> vertices{};
			std::vector<uint32_t> indices{};

			void load_model(const std::string& file_path);
		};

		vk_model(vk_device& device, const builder& builder);
		~vk_model();

		vk_model(const vk_model&) = delete;
		vk_model& operator=(const vk_model&) = delete;

		static std::unique_ptr<vk_model> create_model_from_file(vk_device& device, const std::string& file_path);

		void bind(VkCommandBuffer command_buffer) const;
		void draw(VkCommandBuffer command_buffer) const;

	private:
		void create_vertex_buffers(const std::vector<vertex>& vertices);
		void create_index_buffers(const std::vector<uint32_t>& indices);

		vk_device& device;

		std::unique_ptr<vk_buffer> vertex_buffer{};
		uint32_t vertex_count{};

		std::unique_ptr<vk_buffer> index_buffer{};
		uint32_t index_count{};
		
		bool has_index_buffer{false};
	};
}
