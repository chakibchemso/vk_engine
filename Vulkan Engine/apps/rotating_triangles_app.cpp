#include "rotating_triangles_app.hpp"

#include <future>
#include <glm/glm.hpp>

#include "../engine/vk_model.hpp"
#include "../renderer/vk_device.hpp"
#include "../simple_render_system/vk_simple_render_system.hpp"

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
		const vk_simple_render_system simple_render_system{
			device, renderer.get_swap_chain_render_pass(), nullptr
		}; //TODO
		vk_camera camera{};

		while (!window.should_close())
		{
			glfwPollEvents();

			const float aspect = renderer.get_aspect_ratio();
			camera.set_orthographic_projection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);

			if (const auto command_buffer = renderer.begin_frame())
			{
				const int frame_index = renderer.get_frame_index();
				vk_frame_info frame_info{
					frame_index,
					0,
					command_buffer,
					camera,
					nullptr,
					game_objects
				};
				renderer.begin_swap_chain_render_pass(command_buffer);
				// update rotations
				int i = 0;
				for (auto& [id, game_object] : game_objects)
				{
					i += 1;
					game_object.transform.rotation.z =
						glm::mod<float>(game_object.transform.rotation.z + 0.1f * i, 360.f);
				}
				simple_render_system.render_game_objects(frame_info);
				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.get_device());
	}

	void rotating_triangles_app::load_game_objects()
	{
		std::vector<vk_model::vertex> vertices{
			{{+0.00f, -1.00f, +0.50f}, {1.0f, 1.0, 1.0}},
			{{+0.86f, +0.50f, +0.50f}, {1.0f, 1.0, 1.0}},
			{{-0.86f, +0.50f, +0.50f}, {1.0f, 1.0, 1.0}},
		};

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};

		for (auto& color : colors)
		{
			color = pow(color, glm::vec3{2.2f});
		}

		for (int i = 0; i < 40; i++)
		{
			auto color = colors[i % colors.size()];

			vk_model::builder builder{
				{
					{
						{+0.00f, -1.00f, +0.50f},
						color,
						{0.f, 0.f, 1.f}
					},
					{
						{+0.86f, +0.50f, +0.50f},
						color,
						{0.f, 0.f, 1.f}
					},
					{
						{-0.86f, +0.50f, +0.50f},
						color,
						{0.f, 0.f, 1.f}
					}
				}
			};
			const auto triangle_model = std::make_shared<vk_model>(device, builder);
			auto triangle = vk_game_object::create_game_object();
			triangle.model = triangle_model;
			triangle.transform.scale = glm::vec3(.25f, .25f, .25f) + i * 0.025f;
			triangle.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(45.f)) * glm::vec3(static_cast<float>(i));
			triangle.color = colors[i % colors.size()]; //TODO shit?

			game_objects.emplace(triangle.get_id(), std::move(triangle));
		}
	}
}
