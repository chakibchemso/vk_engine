#pragma once

#include <string>
#include <vector>

namespace vk_engine
{
    class vk_pipeline
    {
    public:
        vk_pipeline(const std::string &vert_shader_path, const std::string &frag_shader_path);

    private:
        static std::vector<char> read_file(const std::string &file_path);

        static void create_graphics_pipeline(const std::string &vert_shader_path, const std::string &frag_shader_path);
    };
}
