#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "vk_simple_render_system.hpp"
#include <glm/glm.hpp>
#include "../engine/vk_model.hpp"
#include "../renderer/vk_device.hpp"

#include <future>
#include <stdexcept>

namespace vk_engine
{
	struct simple_push_const_data
	{
		glm::mat4 model_matrix{1.f};
		glm::mat4 normal_matrix{1.f};
	};

	vk_simple_render_system::vk_simple_render_system(vk_device& device, const VkRenderPass render_pass,
	                                                 VkDescriptorSetLayout global_set_layout) : device{device}
	{
		create_pipeline_layout(global_set_layout);
		create_pipeline(render_pass);
	}

	vk_simple_render_system::~vk_simple_render_system()
	{
		vkDestroyPipelineLayout(device.get_device(), pipeline_layout, nullptr);
	}

	void vk_simple_render_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
	{
		VkPushConstantRange push_constant_range;
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof simple_push_const_data;

		const std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(device.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");
	}

	void vk_simple_render_system::create_pipeline(const VkRenderPass render_pass)
	{
		assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

		pipeline_config_info pipeline_config{};
		vk_pipeline::default_pipeline_config_info(pipeline_config);
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout;
		pipeline = std::make_unique<vk_pipeline>(
			device,
			"assets/shaders/simple_shader.vert.spv",
			"assets/shaders/simple_shader.frag.spv",
			pipeline_config);
	}

	void vk_simple_render_system::render_game_objects(const vk_frame_info& frame_info) const
	{
		pipeline->bind(frame_info.command_buffer);

		vkCmdBindDescriptorSets(
			frame_info.command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout,
			0,
			1,
			&frame_info.global_descriptor_set,
			0,
			nullptr);

		for (auto& [id, game_object] : frame_info.game_objects)
		{
			if (game_object.model == nullptr)
				continue;
			
			simple_push_const_data push{};
			push.model_matrix = game_object.transform.mat4();
			push.normal_matrix = game_object.transform.normal_matrix();

			vkCmdPushConstants(
				frame_info.command_buffer,
				pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof simple_push_const_data,
				&push);

			game_object.model->bind(frame_info.command_buffer);
			game_object.model->draw(frame_info.command_buffer);
		}
	}
}
