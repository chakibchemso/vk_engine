#include "vk_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

vk_engine::vk_pipeline::vk_pipeline(vk_device& device, const std::string& vert_shader_path,
                                    const std::string& frag_shader_path,
                                    const pipeline_config_info& config_info) : vk_device_(device)
{
    create_graphics_pipeline(vert_shader_path, frag_shader_path, config_info);
}

vk_engine::pipeline_config_info vk_engine::vk_pipeline::default_pipeline_config_info(uint32_t width, uint32_t height)
{
    constexpr pipeline_config_info config_info{};

    return config_info;
}

std::vector<char> vk_engine::vk_pipeline::read_file(const std::string& file_path)
{
    std::ifstream file{file_path, std::ios::ate | std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    const size_t file_size = file.tellg();

    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

void vk_engine::vk_pipeline::create_graphics_pipeline(
    const std::string& vert_shader_path,
    const std::string& frag_shader_path,
    const pipeline_config_info&
    config_info)
{
    const auto vert_code = read_file(vert_shader_path);
    const auto frag_code = read_file(frag_shader_path);

    std::cout << "Vertex shader code size: " << vert_code.size() << std::endl;
    std::cout << "Fragment shader code size: " << frag_code.size() << std::endl;
}

void vk_engine::vk_pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(vk_device_.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
}
