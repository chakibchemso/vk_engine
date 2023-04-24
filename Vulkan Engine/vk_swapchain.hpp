#pragma once

#include "vk_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <vector>

namespace vk_engine
{
	class vk_swapchain // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		vk_swapchain(vk_device& device_ref, VkExtent2D window_extent);
		~vk_swapchain();

		vk_swapchain(const vk_swapchain&) = delete;
		void operator=(const vk_swapchain&) = delete;

		VkFramebuffer get_frame_buffer(const int index) const { return swap_chain_framebuffers[index]; }
		VkRenderPass get_render_pass() const { return render_pass; }
		VkImageView get_image_view(const int index) const { return swap_chain_image_views[index]; }
		size_t image_count() const { return swap_chain_images.size(); }
		VkFormat get_swap_chain_image_format() const { return swap_chain_image_format; }
		VkExtent2D get_swap_chain_extent() const { return swap_chain_extent; }
		uint32_t width() const { return swap_chain_extent.width; }
		uint32_t height() const { return swap_chain_extent.height; }

		float extent_aspect_ratio() const
		{
			return static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height);
		}

		VkFormat find_depth_format() const;

		VkResult acquire_next_image(uint32_t* image_index) const;
		VkResult submit_command_buffers(const VkCommandBuffer* buffers, const uint32_t* image_index);

	private:
		void create_swap_chain();
		void create_image_views();
		void create_depth_resources();
		void create_render_pass();
		void create_framebuffers();
		void create_sync_objects();

		// Helper functions
		static VkSurfaceFormatKHR choose_swap_surface_format(
			const std::vector<VkSurfaceFormatKHR>& available_formats);
		static VkPresentModeKHR choose_swap_present_mode(
			const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) const;

		VkFormat swap_chain_image_format;
		VkExtent2D swap_chain_extent{};

		std::vector<VkFramebuffer> swap_chain_framebuffers;
		VkRenderPass render_pass{};

		std::vector<VkImage> depth_images;
		std::vector<VkDeviceMemory> depth_image_memory;
		std::vector<VkImageView> depth_image_views;
		std::vector<VkImage> swap_chain_images;
		std::vector<VkImageView> swap_chain_image_views;

		vk_device& device;
		VkExtent2D window_extent;

		VkSwapchainKHR swap_chain{};

		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> images_in_flight;
		size_t current_frame = 0;
	};
}
