#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "application.hpp"
#include <glm/glm.hpp>
#include "vk_device.hpp"
#include "vk_model.hpp"

#include <array>
#include <future>
#include <iostream>
#include <stdexcept>

namespace vk_engine
{
	struct simple_push_const_data
	{
		glm::mat2 transform{1.0f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	application::application()
	{
		load_game_objects();
		create_pipeline_layout();
		recreate_swap_chain();
		create_command_buffers();
	}

	application::~application()
	{
		vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
	}

	void application::run()
	{
		while (!window.should_close())
		{
			glfwPollEvents();
			draw_frame();
		}

		vkDeviceWaitIdle(device.device());
	}

	void application::load_game_objects()
	{
		std::vector<vk_model::vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};

		const auto triangle_model = std::make_shared<vk_model>(device, vertices);

		// auto triangle = vk_game_object::create_game_object();
		// triangle.model = triangle_model;
		// triangle.color = {.1f, .8f, .1f};
		// triangle.transform2d.translation.x = .2f;
		// triangle.transform2d.scale = {1.f, .5f};
		// triangle.transform2d.rotation = glm::radians(90.f);
		//
		// game_objects.push_back(std::move(triangle));

		for (auto& color : colors)
		{
			color = pow(color, glm::vec3{2.2f});
		}

		for (int i = 0; i < 40; i++)
		{
			auto triangle = vk_game_object::create_game_object();
			triangle.model = triangle_model;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::radians(45.f);
			triangle.color = colors[i % colors.size()];
			
			game_objects.push_back(std::move(triangle));
		}
	}

	void application::create_pipeline_layout()
	{
		VkPushConstantRange push_constant_range;
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof simple_push_const_data;

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");
	}

	void application::create_pipeline()
	{
		assert(swapchain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

		pipeline_config_info pipeline_config{};
		vk_pipeline::default_pipeline_config_info(pipeline_config);
		pipeline_config.render_pass = swapchain->get_render_pass();
		pipeline_config.pipeline_layout = pipeline_layout;
		pipeline = std::make_unique<vk_pipeline>(
			device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipeline_config);
	}

	void application::create_command_buffers()
	{
		command_buffers.resize(swapchain->image_count());

		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandPool = device.get_command_pool();
		allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocate_info, command_buffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers!");
	}

	void application::free_command_buffers()
	{
		vkFreeCommandBuffers(
			device.device(),
			device.get_command_pool(),
			static_cast<float>(command_buffers.size()),
			command_buffers.data());
		command_buffers.clear();
	}

	void application::draw_frame()
	{
		uint32_t image_index;
		auto result = swapchain->acquire_next_image(&image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swap_chain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swap chain image!");

		record_command_buffer(image_index);
		
		result = swapchain->submit_command_buffers(&command_buffers[image_index], &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_window_resized())
		{
			window.reset_window_resized_flag();
			recreate_swap_chain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to present swap chain image!");
	}

	void application::render_game_objects(const VkCommandBuffer command_buffer)
	{
		// update
		int i = 0;
		for (auto& game_object : game_objects)
		{
			i += 1;
			game_object.transform2d.rotation =
				glm::mod<float>(game_object.transform2d.rotation + 0.001f * i, glm::radians(360.f));
		}
		
		pipeline->bind(command_buffer);

		for (auto& game_object : game_objects)
		{
			simple_push_const_data push{};
			push.transform = game_object.transform2d.mat2();
			push.offset = game_object.transform2d.translation;
			push.color = game_object.color;

			vkCmdPushConstants(
				command_buffer,
				pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof simple_push_const_data,
				&push);

			game_object.model->bind(command_buffer);
			game_object.model->draw(command_buffer);
		}
	}

	void application::recreate_swap_chain()
	{
		auto extent = window.get_extent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = window.get_extent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.device());

		if (swapchain == nullptr)
			swapchain = std::make_unique<vk_swapchain>(device, extent);
		else
		{
			swapchain = std::make_unique<vk_swapchain>(device, extent, std::move(swapchain));

			if (swapchain->image_count() != command_buffers.size())
			{
				free_command_buffers();
				create_command_buffers();
			}
		}

		swapchain = std::make_unique<vk_swapchain>(device, extent);

		std::cout << "window size: (h: " << extent.height << ", w: " << extent.width << ')' << std::endl;

		create_pipeline();
	}

	void application::record_command_buffer(const int image_index)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffers[image_index], &begin_info) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer!");

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = swapchain->get_render_pass();
		render_pass_begin_info.framebuffer = swapchain->get_frame_buffer(image_index);

		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = swapchain->get_swap_chain_extent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->get_swap_chain_extent().width);
		viewport.height = static_cast<float>(swapchain->get_swap_chain_extent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		const VkRect2D scissor{{0, 0}, swapchain->get_swap_chain_extent()};
		vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
		vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

		render_game_objects(command_buffers[image_index]);

		vkCmdEndRenderPass(command_buffers[image_index]);

		if (vkEndCommandBuffer(command_buffers[image_index]) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer!");
	}
}
