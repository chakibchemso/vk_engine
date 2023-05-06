#include "rotating_triangles_app.hpp"
#include <glm/glm.hpp>
#include "../engine/vk_model.hpp"
#include "../renderer/vk_device.hpp"
#include "../simple_render_system/vk_simple_render_system.hpp"

#include <future>

namespace vk_engine
{
	rotating_triangles_app::rotating_triangles_app()
	{
		load_game_objects();
	}

	rotating_triangles_app::~rotating_triangles_app()
	= default;

	void rotating_triangles_app::run()
	{
		const vk_simple_render_system simple_render_system{device, renderer.get_swap_chain_render_pass()};

		while (!window.should_close())
		{
			glfwPollEvents();

			if (const auto command_buffer = renderer.begin_frame())
			{
				renderer.begin_swap_chain_render_pass(command_buffer);
				// update rotations
				int i = 0;
				for (auto& game_object : game_objects)
				{
					i += 1;
					game_object.transform.rotation.z =
						glm::mod<float>(game_object.transform.rotation.z + 0.1f * i, 360.f);
				}
				simple_render_system.render_game_objects(command_buffer, game_objects);
				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void rotating_triangles_app::load_game_objects()
	{
		std::vector<vk_model::vertex> vertices{
			{{0.0f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
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
			triangle.transform.scale = glm::vec3(.5f, .5f, 1.f) + i * 0.025f;
			triangle.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(45.f)) * glm::vec3(i);
			triangle.color = colors[i % colors.size()];

			game_objects.push_back(std::move(triangle));
		}
	}
}
