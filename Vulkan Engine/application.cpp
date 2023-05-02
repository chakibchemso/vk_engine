#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "application.hpp"
#include <glm/glm.hpp>
#include "vk_device.hpp"
#include "vk_model.hpp"
#include "vk_simple_render_system.hpp"

#include <future>

namespace vk_engine
{
	application::application()
	{
		load_game_objects();
	}

	application::~application()
	= default;

	void application::run()
	{
		vk_simple_render_system simple_render_system{device, renderer.get_swap_chain_render_pass()};
		
		while (!window.should_close())
		{
			glfwPollEvents();
			
			if (const auto command_buffer = renderer.begin_frame())
			{
				renderer.begin_swap_chain_render_pass(command_buffer);
				simple_render_system.render_game_objects(command_buffer, game_objects);
				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void application::load_game_objects()
	{
		std::vector<vk_model::vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};

		const auto triangle_model = std::make_shared<vk_model>(device, vertices);

		// auto triangle = vk_game_object::create_game_object();
		// triangle.model = triangle_model;
		// triangle.color = {.1f, .8f, .1f};
		// triangle.transform2d.translation.x = .2f;
		// triangle.transform2d.scale = {1.f, .5f};
		// triangle.transform2d.rotation = glm::radians(90.f);
		//
		// game_objects.push_back(std::move(triangle));

		for (auto& color : colors)
		{
			color = pow(color, glm::vec3{2.2f});
		}

		for (int i = 0; i < 40; i++)
		{
			auto triangle = vk_game_object::create_game_object();
			triangle.model = triangle_model;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::radians(45.f);
			triangle.color = colors[i % colors.size()];
			
			game_objects.push_back(std::move(triangle));
		}
	}
}
