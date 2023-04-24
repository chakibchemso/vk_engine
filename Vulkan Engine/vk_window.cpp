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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
    }
}
