#pragma once

#include "vk_window.hpp"
#include "vk_pipeline.hpp"
#include "vk_device.hpp"

namespace vk_engine
{
    class application
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;

        void run();

    private:
        vk_window window_{width, height, "hello Vulkan!"};

        vk_device device_{window_};

        vk_pipeline pipeline_{
            device_,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            vk_pipeline::default_pipeline_config_info(width, height)
        };
    };
}
