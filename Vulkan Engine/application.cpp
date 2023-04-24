#include "application.hpp"

#include <future>
#include <stdexcept>

namespace vk_engine
{
    application::application()
    {
        create_pipeline_layout();
        create_pipeline();
        create_command_buffers();
    }

    application::~application()
    {
        vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
    }

    void application::run()
    {
        while (!window.should_close())
            glfwPollEvents();

    }

    void application::create_pipeline_layout()
    {
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    void application::create_pipeline()
    {
        auto pipeline_config = vk_pipeline::default_pipeline_config_info(swapchain.width(), swapchain.height());
        pipeline_config.render_pass = swapchain.get_render_pass();
        pipeline_config.pipeline_layout = pipeline_layout;
        pipeline = std::make_unique<vk_pipeline>(
            device,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipeline_config);
    }

    void application::create_command_buffers()
    {
    }

    void application::draw_frame()
    {
    }
}
