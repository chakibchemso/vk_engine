#pragma once

#include "../engine/vk_game_object.hpp"
#include "../renderer/vk_device.hpp"
#include "../renderer/vk_renderer.hpp"
#include "../renderer/vk_window.hpp"

namespace vk_engine
{
	class gravity_physics_system
	{
	public:
		explicit gravity_physics_system(float strength);

		const float strength_gravity;

		// dt stands for delta time, and specifies the amount of time to advance the simulation
		// substeps is how many intervals to divide the forward time step in. More substeps result in a
		// more stable simulation, but takes longer to compute
		void update(std::vector<vk_game_object>& objs, float dt, unsigned int substeps = 1) const;

		glm::vec3 compute_force(const vk_game_object& from_obj, const vk_game_object& to_obj) const;

	private:
		void step_simulation(std::vector<vk_game_object>& physics_objs, float dt) const;
	};

	class vec_field_system
	{
	public:
		void update(
			const gravity_physics_system& physics_system,
			const std::vector<vk_game_object>& physics_objs,
			std::vector<vk_game_object>& vector_field) const;
	};


	class gravity_vec_field_app
	{
	public:
		static constexpr int width = 800;
		static constexpr int height = 600;

		gravity_vec_field_app();
		~gravity_vec_field_app();

		gravity_vec_field_app(const gravity_vec_field_app&) = delete;
		gravity_vec_field_app& operator=(const gravity_vec_field_app&) = delete;

		void run();

	private:
		void load_game_objects();

		vk_window window{width, height, "Vulkan!"};
		vk_device device{window};
		vk_renderer renderer{window, device};

		vk_game_object::map game_objects;
	};
}
