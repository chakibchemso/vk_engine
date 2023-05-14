#include "vk_renderer.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <array>
#include <cassert>
#include <future>
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>

#include "vk_device.hpp"

namespace vk_engine
{
	vk_renderer::vk_renderer(vk_window& window, vk_device& device) : window{window}, device{device}
	{
		recreate_swap_chain();
		create_command_buffers();
	}

	vk_renderer::~vk_renderer()
	{
		free_command_buffers();
	}

	VkCommandBuffer vk_renderer::begin_frame()
	{
		assert(!is_frame_started && "Cannot call begin_frame while already in progress.");
		const auto result = swapchain->acquire_next_image(&current_image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swap_chain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swap chain image!");

		is_frame_started = true;
		const auto command_buffer = get_current_command_buffer();
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer!");

		return command_buffer;
	}

	void vk_renderer::end_frame()
	{
		assert(is_frame_started && "Cannot call end_frame while frame is not in progress.");
		const auto command_buffer = get_current_command_buffer();

		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer!");

		if (const auto result = swapchain->submit_command_buffers(&command_buffer, &current_image_index);
			result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_window_resized())
		{
			window.reset_window_resized_flag();
			recreate_swap_chain();
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to present swap chain image!");

		is_frame_started = false;
		current_frame_index = (current_frame_index + 1) % vk_swapchain::MAX_FRAMES_IN_FLIGHT;
	}

	void vk_renderer::begin_swap_chain_render_pass(const VkCommandBuffer command_buffer) const
	{
		assert(is_frame_started && "Cannot call begin_swap_chain_render_pass if frame is not in progress.");
		assert(
			command_buffer == get_current_command_buffer() &&
			"Cannot begin render pass on command buffer from a different frame.");

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = swapchain->get_render_pass();
		render_pass_begin_info.framebuffer = swapchain->get_frame_buffer(current_image_index);

		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = swapchain->get_swap_chain_extent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->get_swap_chain_extent().width);
		viewport.height = static_cast<float>(swapchain->get_swap_chain_extent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		const VkRect2D scissor{{0, 0}, swapchain->get_swap_chain_extent()};
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void vk_renderer::end_swap_chain_render_pass(const VkCommandBuffer command_buffer) const
	{
		assert(is_frame_started && "Cannot call end_swap_chain_render_pass if frame is not in progress.");
		assert(
			command_buffer == get_current_command_buffer() &&
			"Cannot end render pass on command buffer from a different frame.");

		vkCmdEndRenderPass(command_buffer);
	}

	bool vk_renderer::is_frame_in_progress() const
	{
		return is_frame_started;
	}

	int vk_renderer::get_frame_index() const
	{
		assert(is_frame_started && "Cannot get frame index when frame is not in progress.");
		return current_frame_index;
	}

	VkCommandBuffer vk_renderer::get_current_command_buffer() const
	{
		assert(is_frame_started && "Cannot get command buffer when frame is not in progress.");
		return command_buffers[current_frame_index];
	}

	VkRenderPass vk_renderer::get_swap_chain_render_pass() const
	{
		return swapchain->get_render_pass();
	}

	float vk_renderer::get_aspect_ratio() const
	{
		return swapchain->extent_aspect_ratio();
	}

	void vk_renderer::create_command_buffers()
	{
		command_buffers.resize(vk_swapchain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandPool = device.get_command_pool();
		allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocate_info, command_buffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers!");
	}

	void vk_renderer::free_command_buffers()
	{
		vkFreeCommandBuffers(
			device.device(),
			device.get_command_pool(),
			static_cast<float>(command_buffers.size()),
			command_buffers.data());
		command_buffers.clear();
	}

	void vk_renderer::recreate_swap_chain()
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
			const std::shared_ptr old_swap_chain = std::move(swapchain); //old
			swapchain = std::make_unique<vk_swapchain>(device, extent, std::move(swapchain)); //new

			if (!old_swap_chain->compare_swap_formats(*swapchain))
				throw std::runtime_error("Swap chain image or depth format has changed!");
		}

		std::cout
			<< "[RENDERER]" << std::endl
			<< "	window size: (h: " << extent.height << ", w: " << extent.width << ')' << std::endl;
	}
}
