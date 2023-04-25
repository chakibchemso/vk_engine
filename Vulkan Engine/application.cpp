#include "application.hpp"

#include <array>
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
        {
            glfwPollEvents();
            draw_frame();
        }

        vkDeviceWaitIdle(device.device());
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
        command_buffers.resize(swapchain.image_count());

        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandPool = device.get_command_pool();
        allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocate_info, command_buffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");

        for (int i = 0; i < command_buffers.size(); i++)
        {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin recording command buffer!");

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = swapchain.get_render_pass();
            render_pass_begin_info.framebuffer = swapchain.get_frame_buffer(i);

            render_pass_begin_info.renderArea.offset = {0, 0};
            render_pass_begin_info.renderArea.extent = swapchain.get_swap_chain_extent();

            std::array<VkClearValue, 2> clear_values{};
            clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
            clear_values[1].depthStencil = {1.0f, 0};
            render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_begin_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            pipeline->bind(command_buffers[i]);
            vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(command_buffers[i]);
            if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void application::draw_frame()
    {
        uint32_t image_index;
        auto result = swapchain.acquire_next_image(&image_index);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Failed to acquire swap chain image!");

        result = swapchain.submit_command_buffers(&command_buffers[image_index], &image_index);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Failed to present swap chain image!");
    }
}
