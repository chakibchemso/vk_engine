#include "gravity_vec_field_app.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "../simple_render_system/vk_simple_render_system.hpp"

namespace vk_engine
{
	// Note: also would need to add RigidBody2dComponent to game object
	// struct RigidBody2dComponent {
	//   glm::vec2 velocity;
	//   float mass{1.0f};
	// };

	class gravity_physics_system
	{
	public:
		explicit gravity_physics_system(const float strength) : strength_gravity{strength}
		{
		}

		const float strength_gravity;

		// dt stands for delta time, and specifies the amount of time to advance the simulation
		// substeps is how many intervals to divide the forward time step in. More substeps result in a
		// more stable simulation, but takes longer to compute
		void update(std::vector<vk_game_object>& objs, const float dt, const unsigned int substeps = 1) const
		{
			const float step_delta = dt / substeps;
			for (int i = 0; i < substeps; i++)
			{
				step_simulation(objs, step_delta);
			}
		}

		glm::vec3 compute_force(const vk_game_object& from_obj, const vk_game_object& to_obj) const
		{
			const auto offset = from_obj.transform.translation - to_obj.transform.translation;
			const float distance_squared = dot(offset, offset);

			// clown town - just going to return 0 if objects are too close together...
			if (glm::abs(distance_squared) < 1e-10f)
			{
				return {.0f, .0f, 0.f};
			}

			const float force =
				strength_gravity * to_obj.rigid_body.mass * from_obj.rigid_body.mass / distance_squared;
			return force * offset / glm::sqrt(distance_squared);
		}

	private:
		void step_simulation(std::vector<vk_game_object>& physics_objs, const float dt) const
		{
			// Loops through all pairs of objects and applies attractive force between them
			for (auto iter_a = physics_objs.begin(); iter_a != physics_objs.end(); ++iter_a)
			{
				auto& obj_a = *iter_a;
				for (auto iter_b = iter_a; iter_b != physics_objs.end(); ++iter_b)
				{
					if (iter_a == iter_b) continue;
					auto& obj_b = *iter_b;

					auto force = compute_force(obj_a, obj_b);
					obj_a.rigid_body.velocity += dt * -force / obj_a.rigid_body.mass;
					obj_b.rigid_body.velocity += dt * force / obj_b.rigid_body.mass;
				}
			}

			// update each objects position based on its final velocity
			for (auto& obj : physics_objs)
			{
				obj.transform.translation += dt * obj.rigid_body.velocity;
			}
		}
	};

	class vec2_field_system
	{
	public:
		void update(
			const gravity_physics_system& physics_system,
			const std::vector<vk_game_object>& physics_objs,
			std::vector<vk_game_object>& vector_field) const
		{
			// For each field line we calculate the net gravitation force for that point in space
			for (auto& vf : vector_field)
			{
				glm::vec3 direction{};
				for (auto& obj : physics_objs)
				{
					direction += physics_system.compute_force(obj, vf);
				}

				// This scales the length of the field line based on the log of the length
				// values were chosen just through trial and error based on what i liked the look
				// of and then the field line is rotated to point in the direction of the field
				vf.transform.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(length(direction) + 1) / 3.f, 0.f, .5f);
				vf.transform.rotation.z = atan2(direction.y, direction.x) * 180.f / glm::pi<float>();
			}
		}
	};

	std::unique_ptr<vk_model> create_square_model(vk_device& device, const glm::vec3 offset)
	{
		vk_model::builder builder{};
		builder.vertices = {
			{{-0.5f, -0.5f, .0f}},
			{{0.5f, 0.5f, .0f}},
			{{-0.5f, 0.5f, .0f}},
			{{-0.5f, -0.5f, .0f}},
			{{0.5f, -0.5f, .0f}},
			{{0.5f, 0.5f, .0f}},
		};
		for (auto& [position, color] : builder.vertices)
		{
			position += offset;
		}
		return std::make_unique<vk_model>(device, builder);
	}

	std::unique_ptr<vk_model> create_circle_model(vk_device& device, const unsigned int num_sides)
	{
		std::vector<vk_model::vertex> unique_vertices{};
		for (int i = 0; i < num_sides; i++)
		{
			const float angle = i * glm::two_pi<float>() / num_sides;
			unique_vertices.push_back({{glm::cos(angle), glm::sin(angle), .0f}});
		}
		unique_vertices.push_back({}); // adds center vertex at 0, 0, 0

		std::vector<vk_model::vertex> vertices{};
		for (int i = 0; i < num_sides; i++)
		{
			vertices.push_back(unique_vertices[i]);
			vertices.push_back(unique_vertices[(i + 1) % num_sides]);
			vertices.push_back(unique_vertices[num_sides]);
		}
		vk_model::builder builder{vertices};
		return std::make_unique<vk_model>(device, builder);
	}

	gravity_vec_field_app::gravity_vec_field_app() { load_game_objects(); }

	gravity_vec_field_app::~gravity_vec_field_app() = default;

	void gravity_vec_field_app::run()
	{
		// create some models
		std::shared_ptr<vk_model> square_model = create_square_model(
			device,
			{.5f, .0f, 0.f}); // offset model by .5 so rotation occurs at edge rather than center of square
		std::shared_ptr<vk_model> circle_model = create_circle_model(device, 64);

		// create physics objects
		std::vector<vk_game_object> physics_objects{};
		auto red = vk_game_object::create_game_object();
		red.transform.scale = glm::vec3{.05f};
		red.transform.translation = {.5f, .5f, .0f};
		red.color = {1.f, 0.f, 0.f};
		red.rigid_body.velocity = {-.5f, .0f, .0f};
		red.model = circle_model;
		physics_objects.push_back(std::move(red));
		auto blue = vk_game_object::create_game_object();
		blue.transform.scale = glm::vec3{.05f};
		blue.transform.translation = {-.45f, -.25f, .0f};
		blue.color = {0.f, 0.f, 1.f};
		blue.rigid_body.velocity = {.5f, .0f, .0f};
		blue.model = circle_model;
		physics_objects.push_back(std::move(blue));

		// create vector field
		std::vector<vk_game_object> vector_field{};
		int grid_count = 40;
		for (int i = 0; i < grid_count; i++)
		{
			for (int j = 0; j < grid_count; j++)
			{
				auto vf = vk_game_object::create_game_object();
				vf.transform.scale = glm::vec3(0.005f);
				vf.transform.translation = {
					-1.0f + (i + 0.5f) * 2.0f / grid_count,
					-1.0f + (j + 0.5f) * 2.0f / grid_count,
					.0f
				};
				vf.color = glm::vec3(1.0f);
				vf.model = square_model;
				vector_field.push_back(std::move(vf));
			}
		}

		gravity_physics_system gravity_system{0.81f};
		vec2_field_system vec_field_system{};

		vk_simple_render_system simple_render_system{device, renderer.get_swap_chain_render_pass()};
		vk_camera camera{};

		while (!window.should_close())
		{
			glfwPollEvents();

			const float aspect = renderer.get_aspect_ratio();
			camera.set_orthographic_projection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);

			if (auto command_buffer = renderer.begin_frame())
			{
				// update systems
				gravity_system.update(physics_objects, 1.f / 60, 5);
				vec_field_system.update(gravity_system, physics_objects, vector_field);

				// render system
				renderer.begin_swap_chain_render_pass(command_buffer);
				simple_render_system.render_game_objects(command_buffer, physics_objects, camera);
				simple_render_system.render_game_objects(command_buffer, vector_field, camera);
				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void gravity_vec_field_app::load_game_objects()
	{
		std::vector<vk_model::vertex> vertices{
			{{0.0f, -0.5f, .0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, .0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, .0f}, {0.0f, 0.0f, 1.0f}}
		};
		vk_model::builder builder{vertices};
		const auto lve_model = std::make_shared<vk_model>(device, builder);

		auto triangle = vk_game_object::create_game_object();
		triangle.model = lve_model;
		triangle.color = {.1f, .8f, .1f};
		triangle.transform.translation.x = .2f;
		triangle.transform.scale = {2.f, .5f, 1.f};
		triangle.transform.rotation.z = .25f * glm::two_pi<float>();

		game_objects.push_back(std::move(triangle));
	}
} // namespace lve
