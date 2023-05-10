#pragma once

#define GLFW_INCLUDE_VULKAN

#include <string>
#include <GLFW/glfw3.h>

namespace vk_engine
{
	class vk_window
	{
	public:
		vk_window(int width, int height, std::string name);

		~vk_window();

		vk_window(const vk_window&) = delete;
		vk_window& operator=(const vk_window&) = delete;

		bool should_close() const;
		bool was_window_resized() const;
		void reset_window_resized_flag();
		VkExtent2D get_extent() const;
		GLFWwindow* get_glfw_window() const;
		void create_window_surface(VkInstance instance, VkSurfaceKHR* surface) const;

	private:
		void init_window();
		static void frame_buffer_resize_callback(GLFWwindow* window, int width, int height);

		int width{};
		int height{};
		bool frame_buffer_resized = false;

		std::string window_name;
		GLFWwindow* window;
	};
}
