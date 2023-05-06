#include "application.hpp"
#include <glm/glm.hpp>
#include "../engine/vk_model.hpp"
#include "../renderer/vk_device.hpp"
#include "../simple_render_system/vk_simple_render_system.hpp"

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
					game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y + 0.1f, 360.f);
					game_object.transform.rotation.x = glm::mod(game_object.transform.rotation.x + 0.1f, 360.f);
					game_object.transform.rotation.z = glm::mod(game_object.transform.rotation.z + 0.1f, 360.f);
				}
				simple_render_system.render_game_objects(command_buffer, game_objects);
				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<vk_model> create_cube_model(vk_device& device, const glm::vec3 offset)
	{
		std::vector<vk_model::vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& [position, color] : vertices)
		{
			position += offset;
		}
		return std::make_unique<vk_model>(device, vertices);
	}

	void application::load_game_objects()
	{
		std::shared_ptr<vk_model> model = create_cube_model(device, {0.f, 0.f, 0.f});

		auto cube = vk_game_object::create_game_object();
		cube.model = model;
		cube.transform.translation = {.0f, .0f, .5f};
		cube.transform.scale = {.5f, .5f, .5f};
		game_objects.push_back(std::move(cube));
	}
}
