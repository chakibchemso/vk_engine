#pragma once

#include "vk_window.hpp"
#include "vk_pipeline.hpp"

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
        vk_pipeline pipeline_{"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};
    };
    
}
