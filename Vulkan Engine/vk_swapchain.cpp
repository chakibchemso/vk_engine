#include "vk_swapchain.hpp"

// std
#include <array>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace vk_engine
{
	vk_swapchain::vk_swapchain(vk_device& device_ref, const VkExtent2D window_extent)
		: device{device_ref}, window_extent{window_extent}
	{
		create_swap_chain();
		create_image_views();
		create_render_pass();
		create_depth_resources();
		create_framebuffers();
		create_sync_objects();
	}

	vk_swapchain::~vk_swapchain()
	{
		for (const auto image_view : swap_chain_image_views)
		{
			vkDestroyImageView(device.device(), image_view, nullptr);
		}
		swap_chain_image_views.clear();

		if (swap_chain != nullptr)
		{
			vkDestroySwapchainKHR(device.device(), swap_chain, nullptr);
			swap_chain = nullptr;
		}

		for (int i = 0; i < depth_images.size(); i++)
		{
			vkDestroyImageView(device.device(), depth_image_views[i], nullptr);
			vkDestroyImage(device.device(), depth_images[i], nullptr);
			vkFreeMemory(device.device(), depth_image_memory[i], nullptr);
		}

		for (const auto framebuffer : swap_chain_framebuffers)
		{
			vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(device.device(), render_pass, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device.device(), render_finished_semaphores[i], nullptr);
			vkDestroySemaphore(device.device(), image_available_semaphores[i], nullptr);
			vkDestroyFence(device.device(), in_flight_fences[i], nullptr);
		}
	}

	VkResult vk_swapchain::acquire_next_image(uint32_t* image_index) const
	{
		vkWaitForFences(
			device.device(),
			1,
			&in_flight_fences[current_frame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		const VkResult result = vkAcquireNextImageKHR(
			device.device(),
			swap_chain,
			std::numeric_limits<uint64_t>::max(),
			image_available_semaphores[current_frame], // must be a not signaled semaphore
			VK_NULL_HANDLE,
			image_index);

		return result;
	}

	VkResult vk_swapchain::submit_command_buffers(
		const VkCommandBuffer* buffers, const uint32_t* image_index)
	{
		if (images_in_flight[*image_index] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device.device(), 1, &images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
		}
		images_in_flight[*image_index] = in_flight_fences[current_frame];

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		const VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
		constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = buffers;

		const VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkResetFences(device.device(), 1, &in_flight_fences[current_frame]);
		if (vkQueueSubmit(device.graphics_queue(), 1, &submit_info, in_flight_fences[current_frame]) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		const VkSwapchainKHR swap_chains[] = {swap_chain};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;

		present_info.pImageIndices = image_index;

		const auto result = vkQueuePresentKHR(device.present_queue(), &present_info);

		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void vk_swapchain::create_swap_chain()
	{
		auto [capabilities, formats, present_modes] = device.get_swap_chain_support();

		const auto [format, colorSpace] = choose_swap_surface_format(formats);
		const VkPresentModeKHR present_mode = choose_swap_present_mode(present_modes);
		const VkExtent2D extent = choose_swap_extent(capabilities);

		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 &&
			image_count > capabilities.maxImageCount)
		{
			image_count = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = device.surface();

		create_info.minImageCount = image_count;
		create_info.imageFormat = format;
		create_info.imageColorSpace = colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto [graphics_family, present_family, graphics_family_has_value, present_family_has_value] = device.
			find_physical_queue_families();
		const uint32_t queue_family_indices[] = {graphics_family, present_family};

		if (graphics_family != present_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0; // Optional
			create_info.pQueueFamilyIndices = nullptr; // Optional
		}

		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device.device(), &create_info, nullptr, &swap_chain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		// we only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(device.device(), swap_chain, &image_count, nullptr);
		swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(device.device(), swap_chain, &image_count, swap_chain_images.data());

		swap_chain_image_format = format;
		swap_chain_extent = extent;
	}

	void vk_swapchain::create_image_views()
	{
		swap_chain_image_views.resize(swap_chain_images.size());
		for (size_t i = 0; i < swap_chain_images.size(); i++)
		{
			VkImageViewCreateInfo view_info{};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = swap_chain_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = swap_chain_image_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device(), &view_info, nullptr, &swap_chain_image_views[i]) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void vk_swapchain::create_render_pass()
	{
		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = find_depth_format();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref;
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription color_attachment = {};
		color_attachment.format = get_swap_chain_image_format();
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref;
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		const std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &render_pass_info, nullptr, &render_pass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void vk_swapchain::create_framebuffers()
	{
		swap_chain_framebuffers.resize(image_count());
		for (size_t i = 0; i < image_count(); i++)
		{
			std::array<VkImageView, 2> attachments = {swap_chain_image_views[i], depth_image_views[i]};

			const auto [width, height] = get_swap_chain_extent();
			VkFramebufferCreateInfo framebuffer_info = {};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = render_pass;
			framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebuffer_info.pAttachments = attachments.data();
			framebuffer_info.width = width;
			framebuffer_info.height = height;
			framebuffer_info.layers = 1;

			if (vkCreateFramebuffer(
				device.device(),
				&framebuffer_info,
				nullptr,
				&swap_chain_framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void vk_swapchain::create_depth_resources()
	{
		const VkFormat depth_format = find_depth_format();
		auto [width, height] = get_swap_chain_extent();

		depth_images.resize(image_count());
		depth_image_memory.resize(image_count());
		depth_image_views.resize(image_count());

		for (int i = 0; i < depth_images.size(); i++)
		{
			VkImageCreateInfo image_info{};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.extent.width = width;
			image_info.extent.height = height;
			image_info.extent.depth = 1;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = depth_format;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.flags = 0;

			device.create_image_with_info(
				image_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				depth_images[i],
				depth_image_memory[i]);

			VkImageViewCreateInfo view_info{};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = depth_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = depth_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device(), &view_info, nullptr, &depth_image_views[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void vk_swapchain::create_sync_objects()
	{
		image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
		images_in_flight.resize(image_count(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device.device(), &semaphore_info, nullptr, &image_available_semaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(device.device(), &semaphore_info, nullptr, &render_finished_semaphores[i]) !=
				VK_SUCCESS ||
				vkCreateFence(device.device(), &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkSurfaceFormatKHR vk_swapchain::choose_swap_surface_format(
		const std::vector<VkSurfaceFormatKHR>& available_formats)
	{
		for (const auto& available_format : available_formats)
		{
			if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return available_format;
			}
		}

		return available_formats[0];
	}

	VkPresentModeKHR vk_swapchain::choose_swap_present_mode(
		const std::vector<VkPresentModeKHR>& available_present_modes)
	{
		for (const auto& available_present_mode : available_present_modes)
		{
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				std::cout << "Present mode: Mailbox" << std::endl;
				return available_present_mode;
			}
		}

		// for (const auto &availablePresentMode : availablePresentModes) {
		//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//     std::cout << "Present mode: Immediate" << std::endl;
		//     return availablePresentMode;
		//   }
		// }

		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D vk_swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		VkExtent2D actual_extent = window_extent;
		actual_extent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}

	VkFormat vk_swapchain::find_depth_format() const
	{
		return device.find_supported_format(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
} // namespace lve
