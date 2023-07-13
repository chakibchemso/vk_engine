#pragma once

#include "vk_pipeline.hpp"
#include "../../engine/vk_frame_info.hpp"
#include "../../renderer/vk_device.hpp"

#include <memory>

namespace vk_engine
{
	class vk_point_light_system
	{
	public:
		vk_point_light_system(vk_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~vk_point_light_system();

		vk_point_light_system(const vk_point_light_system&) = delete;
		vk_point_light_system& operator=(const vk_point_light_system&) = delete;

		void render_light(const vk_frame_info& frame_info) const;

	private:
		void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
		void create_pipeline(VkRenderPass render_pass);

		vk_device& device;

		std::unique_ptr<vk_pipeline> pipeline;

		VkPipelineLayout pipeline_layout{};
	};
}
