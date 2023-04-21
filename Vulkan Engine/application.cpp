#include "application.hpp"

namespace vk_engine
{
    void application::run()
    {
        while (!window_.should_close())
        {
            glfwPollEvents();
        }
    }
}
