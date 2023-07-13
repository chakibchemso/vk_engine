#include "application.hpp"

#include <chrono>
#include <future>
#include <glm/glm.hpp>

#include "input_controller.hpp"
#include "../engine/vk_camera.hpp"
#include "../engine/vk_model.hpp"
#include "../renderer/vk_buffer.hpp"
#include "../renderer/vk_device.hpp"
#include "../renderer/simple_render_system/vk_point_light_system.hpp"
#include "../renderer/simple_render_system/vk_simple_render_system.hpp"

using vk_engine::application;

struct global_ubo
{
	alignas(16) glm::mat4 projection{1.f};
	alignas(16) glm::mat4 view{1.f};
	alignas(16) glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .2f};
	alignas(16) glm::vec3 light_direction{
		normalize(glm::vec3{1.f, -3.f, -1.f})
	};
	alignas(16) glm::vec3 point_light_position{-1.f};
	alignas(16) glm::vec4 point_light_color{1.f, 1.f, 0.f, 1.f};
};

application::application()
{
	global_pool = vk_descriptor_pool::builder(device)
	              .set_max_sets(vk_swapchain::MAX_FRAMES_IN_FLIGHT)
	              .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vk_swapchain::MAX_FRAMES_IN_FLIGHT)
	              .build();
	load_game_objects();
}

application::~application() = default;

void application::run()
{
	std::vector<std::unique_ptr<vk_buffer>> ubo_buffers(vk_swapchain::MAX_FRAMES_IN_FLIGHT);

	for (auto& ubo_buffer : ubo_buffers)
	{
		ubo_buffer = std::make_unique<vk_buffer>(
			device,
			sizeof global_ubo,
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		ubo_buffer->map();
	}

	auto global_set_layout = vk_descriptor_set_layout::builder(device)
	                         .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
	                         .build();

	std::vector<VkDescriptorSet> global_descriptor_sets(vk_swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < static_cast<int>(global_descriptor_sets.size()); ++i)
	{
		auto buffer_info = ubo_buffers[i]->descriptor_info();
		vk_descriptor_writer(*global_set_layout, *global_pool)
			.write_buffer(0, &buffer_info)
			.build(global_descriptor_sets[i]);
	}

	global_ubo ubo{};
	auto viewer_object = vk_game_object::create_game_object();
	viewer_object.transform.translation = {0.f, -1.f, -2.5f};
	viewer_object.transform.rotation.x = -.5f;

	constexpr input_controller cam_controller{};

	vk_camera camera{};
	//camera.set_view_direction(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
	//camera.set_view_target(glm::vec3(-1, -2, -2), glm::vec3(0, 0, 2.5));

	const vk_simple_render_system simple_render_system{
		device, renderer.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()
	};

	const vk_point_light_system point_light_system{
		device, renderer.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()
	};

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
				camera,
				global_descriptor_sets[frame_index],
				game_objects,
			};
			// std::cout
			// 	<< "[Main]" << std::endl
			// 	<< "	frame index:" << frame_index << std::endl
			// 	<< "	frame time: " << frame_time << std::endl
			// 	<< "	frame rate: " << 1 / frame_time << std::endl;

			//update
			ubo.projection = camera.get_projection();
			ubo.view = camera.get_view();
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

			simple_render_system.render_game_objects(frame_info);
			point_light_system.render_light(frame_info);

			renderer.end_swap_chain_render_pass(command_buffer);
			renderer.end_frame();
		}
	}

	vkDeviceWaitIdle(device.get_device());
}

void application::load_game_objects()
{
	const std::shared_ptr flat_vase_model = vk_model::create_model_from_file(
		device,
		R"(assets\models\flat_vase.obj)");

	const std::shared_ptr smooth_vase_model = vk_model::create_model_from_file(
		device,
		R"(assets\models\smooth_vase.obj)");

	const std::shared_ptr floor_model = vk_model::create_model_from_file(
		device,
		R"(assets\models\quad.obj)");

	const std::shared_ptr raiju_model = vk_model::create_model_from_file(
		device,
		R"(assets\models\raiju.obj)");

	auto flat_vase_object = vk_game_object::create_game_object();
	flat_vase_object.model = flat_vase_model;
	flat_vase_object.transform.translation = {-.5f, .0f, .0f};
	flat_vase_object.transform.scale = glm::vec3{3.f, 2.0f, 3.0f};
	game_objects.emplace(flat_vase_object.get_id(), std::move(flat_vase_object));

	auto smooth_vase_object = vk_game_object::create_game_object();
	smooth_vase_object.model = smooth_vase_model;
	smooth_vase_object.transform.translation = {.5f, .0f, .0f};
	smooth_vase_object.transform.scale = glm::vec3{3.0f, 1.0f, 3.0f};
	game_objects.emplace(smooth_vase_object.get_id(), std::move(smooth_vase_object));

	auto floor_object = vk_game_object::create_game_object();
	floor_object.model = floor_model;
	floor_object.transform.translation = {.0f, .0f, .0f};
	floor_object.transform.scale = glm::vec3{3.0f, 1.0f, 3.0f};
	game_objects.emplace(floor_object.get_id(), std::move(floor_object));

	auto raiju_object = vk_game_object::create_game_object();
	raiju_object.model = raiju_model;
	raiju_object.transform.translation = {.0f, -1.0f, .0f};
	raiju_object.transform.scale = glm::vec3{.1f, -.1f, .1f};
	game_objects.emplace(raiju_object.get_id(), std::move(raiju_object));
}
