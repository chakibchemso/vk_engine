#include "vk_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

vk_engine::vk_pipeline::vk_pipeline(const std::string& vert_shader_path, const std::string& frag_shader_path)
{
    create_graphics_pipeline(vert_shader_path, frag_shader_path);
}

std::vector<char> vk_engine::vk_pipeline::read_file(const std::string& file_path)
{
    std::ifstream file{file_path, std::ios::ate | std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    const size_t file_size = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

void vk_engine::vk_pipeline::create_graphics_pipeline(const std::string& vert_shader_path,const std::string& frag_shader_path)
{
    const auto vert_code = read_file(vert_shader_path);
    const auto frag_code = read_file(frag_shader_path);

    std::cout << "Vertex shader code size: " << vert_code.size() << std::endl;
    std::cout << "Fragment shader code size: " << frag_code.size() << std::endl;
}
