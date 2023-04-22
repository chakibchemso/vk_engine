#include "vk_window.hpp"

#include <stdexcept>

namespace vk_engine
{
    vk_window::vk_window(const int width, const int height, std::string name)
        : width_(width), height_(height), window_name_(std::move(name))
    {
        init_window();
    }

    vk_window::~vk_window()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    bool vk_window::should_close() const
    {
        return glfwWindowShouldClose(window_);
    }

    void vk_window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface!");
    }

    void vk_window::init_window()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
    }
}
