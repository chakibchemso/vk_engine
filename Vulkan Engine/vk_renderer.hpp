#pragma once

#include "vk_window.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"

#include <memory>

namespace vk_engine
{
	class vk_renderer
	{
	public:
		vk_renderer(vk_window& window, vk_device& device);
		~vk_renderer();

		vk_renderer(const vk_renderer&) = delete;
		vk_renderer& operator=(const vk_renderer&) = delete;

		VkCommandBuffer begin_frame();
		void end_frame();
		void begin_swap_chain_render_pass(VkCommandBuffer command_buffer) const;
		void end_swap_chain_render_pass(VkCommandBuffer command_buffer) const;
		bool is_frame_in_progress() const;
		int get_frame_index() const;
		VkCommandBuffer get_current_command_buffer() const;
		VkRenderPass get_swap_chain_render_pass() const;

	private:
		void create_command_buffers();
		void free_command_buffers();

		void recreate_swap_chain();

		vk_window& window;
		vk_device& device;

		std::unique_ptr<vk_swapchain> swapchain;

		std::vector<VkCommandBuffer> command_buffers;

		uint32_t current_image_index;
		int current_frame_index;
		bool is_frame_started{false};
	};
}
