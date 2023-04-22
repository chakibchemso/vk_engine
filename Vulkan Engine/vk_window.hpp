#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

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

        void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        void init_window();

        const int width_{};
        const int height_{};

        std::string window_name_;
        GLFWwindow* window_;
    };
}
