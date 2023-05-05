#pragma once

#include <string>
#include <vector>
#include "../renderer/vk_device.hpp"

namespace vk_engine
{
	struct pipeline_config_info
	{
		pipeline_config_info(const pipeline_config_info&) = delete;
		pipeline_config_info& operator=(const pipeline_config_info&) = delete;

		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t sub_pass = 0;
	};

	class vk_pipeline
	{
	public:
		vk_pipeline(
			vk_device& device,
			const std::string& vert_shader_path,
			const std::string& frag_shader_path,
			const pipeline_config_info& config_info);

		~vk_pipeline();

		vk_pipeline(const vk_pipeline&) = delete;
		vk_pipeline& operator=(const vk_pipeline&) = delete;

		void bind(VkCommandBuffer command_buffer);

		static void default_pipeline_config_info(pipeline_config_info& config_info);

	private:
		static std::vector<char> read_file(const std::string& file_path);

		void create_graphics_pipeline(
			const std::string& vert_shader_path,
			const std::string& frag_shader_path,
			const pipeline_config_info& config_info);

		void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module) const;

		vk_device& device;

		VkPipeline graphics_pipeline{};

		VkShaderModule vert_shader_module{};
		VkShaderModule frag_shader_module{};
	};
}
