#include "vk_window.hpp"

#include <stdexcept>

namespace vk_engine
{
	vk_window::vk_window(const int width, const int height, std::string name)
		: width(width), height(height), window_name(std::move(name))
	{
		init_window();
	}

	vk_window::~vk_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool vk_window::should_close() const
	{
		return glfwWindowShouldClose(window);
	}

	bool vk_window::was_window_resized() const
	{
		return frame_buffer_resized;
	}

	void vk_window::reset_window_resized_flag()
	{
		frame_buffer_resized = false;
	}

	VkExtent2D vk_window::get_extent() const
	{
		return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	}

	void vk_window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface) const
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
	}

	void vk_window::init_window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);
	}

	void vk_window::frame_buffer_resize_callback(GLFWwindow* window, const int width, const int height)
	{
		const auto new_vk_window = static_cast<vk_window*>(glfwGetWindowUserPointer(window));
		new_vk_window->frame_buffer_resized = true;
		new_vk_window->width = width;
		new_vk_window->height = height;
	}
}
