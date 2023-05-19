#include "application.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <glm/glm.hpp>

#include "input_controller.hpp"
#include "../engine/vk_camera.hpp"
#include "../engine/vk_model.hpp"
#include "../renderer/vk_buffer.hpp"
#include "../renderer/vk_device.hpp"
#include "../simple_render_system/vk_simple_render_system.hpp"

namespace vk_engine
{
	struct global_ubo
	{
		glm::mat4 projection_view{1.f};
		glm::vec3 light_direction{
			normalize(glm::vec3{1.f, -3.f, -1.f})
		};
	};

	application::application()
	{
		load_game_objects();
	}

	application::~application()
	= default;

	void application::run()
	{
		std::vector<std::unique_ptr<vk_buffer>> ubo_buffers(vk_swapchain::MAX_FRAMES_IN_FLIGHT);

		for (auto& ubo_buffer : ubo_buffers)
		{
			ubo_buffer = std::make_unique<vk_buffer>(
				device,
				sizeof global_ubo,
				1,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			ubo_buffer->map();
		}

		auto viewer_object = vk_game_object::create_game_object();

		constexpr input_controller cam_controller{};

		vk_camera camera{};
		//camera.set_view_direction(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
		//camera.set_view_target(glm::vec3(-1, -2, -2), glm::vec3(0, 0, 2.5));

		const vk_simple_render_system simple_render_system{device, renderer.get_swap_chain_render_pass()};

		auto current_time = std::chrono::high_resolution_clock::now();

		while (!window.should_close())
		{
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).
				count();
			current_time = new_time;

			cam_controller.move_in_plane_xz(window.get_glfw_window(), frame_time, viewer_object);
			camera.set_view_yxz(viewer_object.transform.translation, viewer_object.transform.rotation);

			const float aspect = renderer.get_aspect_ratio();
			camera.set_orthographic_projection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);
			camera.set_perspective_projection(glm::radians(60.f), aspect, 0.1f, 100.f);

			if (const auto command_buffer = renderer.begin_frame())
			{
				int frame_index = renderer.get_frame_index();
				vk_frame_info frame_info{
					frame_index,
					frame_time,
					command_buffer,
					camera
				};
				// std::cout
				// 	<< "[Main]" << std::endl
				// 	<< "	frame index:" << frame_index << std::endl
				// 	<< "	frame time: " << frame_time << std::endl
				// 	<< "	frame rate: " << 1 / frame_time << std::endl;

				//update
				global_ubo ubo{};
				ubo.projection_view = camera.get_projection() * camera.get_view();
				ubo_buffers[frame_index]->write_to_buffer(&ubo);
				ubo_buffers[frame_index]->flush();

				//render
				renderer.begin_swap_chain_render_pass(command_buffer);

				// update rotations
				/*for (auto& game_object : game_objects)
				{
					game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y + 0.1f, 360.f);
					game_object.transform.rotation.x = glm::mod(game_object.transform.rotation.x + 0.1f, 360.f);
					game_object.transform.rotation.z = glm::mod(game_object.transform.rotation.z + 0.1f, 360.f);
				}*/

				simple_render_system.render_game_objects(frame_info, game_objects);

				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.get_device());
	}

	void application::load_game_objects()
	{
		const std::shared_ptr model = vk_model::create_model_from_file(
			device,
			R"(assets\models\smooth_vase.obj)");

		auto game_object = vk_game_object::create_game_object();
		game_object.model = model;
		game_object.transform.translation = {.0f, .0f, 2.5f};
		game_object.transform.scale = glm::vec3{3.f};
		game_objects.push_back(std::move(game_object));
	}
}
