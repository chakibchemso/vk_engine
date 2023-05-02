#pragma once

#include "vk_device.hpp"
#include "vk_game_object.hpp"
#include "vk_pipeline.hpp"

#include <memory>

namespace vk_engine
{
	class vk_simple_render_system
	{
	public:
		vk_simple_render_system(vk_device& device, VkRenderPass render_pass);
		~vk_simple_render_system();

		vk_simple_render_system(const vk_simple_render_system&) = delete;
		vk_simple_render_system& operator=(const vk_simple_render_system&) = delete;
		
		void render_game_objects(VkCommandBuffer command_buffer, std::vector<vk_game_object>& game_objects) const;

	private:
		void create_pipeline_layout();
		void create_pipeline(VkRenderPass render_pass);
		
		vk_device& device;
		
		std::unique_ptr<vk_pipeline> pipeline;

		VkPipelineLayout pipeline_layout{};
	};
}
