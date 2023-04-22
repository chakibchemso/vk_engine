#pragma once

#include "vk_device.hpp"
#include <string>
#include <vector>

namespace vk_engine
{
    struct pipeline_config_info
    {
    };

    class vk_pipeline
    {
    public:
        vk_pipeline(
            vk_device& device,
            const std::string& vert_shader_path,
            const std::string& frag_shader_path,
            const pipeline_config_info& config_info);

        ~vk_pipeline() = default;

        vk_pipeline(const vk_pipeline&) = delete;
        void operator=(const vk_pipeline&) = delete;

        static pipeline_config_info default_pipeline_config_info(uint32_t width, uint32_t height);

    private:
        static std::vector<char> read_file(const std::string& file_path);

        static void create_graphics_pipeline(
            const std::string& vert_shader_path,
            const std::string& frag_shader_path,
            const pipeline_config_info& config_info);

        void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

        vk_device& vk_device_;
        VkPipeline graphics_pipeline_{};
        VkShaderModule vert_shader_module_{};
        VkShaderModule frag_shader_module_{};
    };
}
