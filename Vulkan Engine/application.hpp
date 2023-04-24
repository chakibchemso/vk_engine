#pragma once

#include "vk_window.hpp"
#include "vk_pipeline.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"

#include <memory>

namespace vk_engine
{
	class application
	{
	public:
		static constexpr int width = 800;
		static constexpr int height = 600;

		application();
		~application();

		application(const application&) = delete;
		application& operator=(const application&) = delete;

		void run();

	private:
		void create_pipeline_layout();
		void create_pipeline();

		void create_command_buffers();

		void draw_frame();

		vk_window window{width, height, "hello Vulkan!"};

		vk_device device{window};

		vk_swapchain swapchain{device, window.get_extent()};

		std::unique_ptr<vk_pipeline> pipeline;

		VkPipelineLayout pipeline_layout;

		std::vector<VkCommandBuffer> command_buffers;
	};
}
