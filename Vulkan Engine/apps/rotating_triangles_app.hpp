#pragma once

#include "../engine/vk_game_object.hpp"
#include "../renderer/vk_device.hpp"
#include "../renderer/vk_renderer.hpp"
#include "../renderer/vk_window.hpp"

namespace vk_engine
{
	class rotating_triangles_app
	{
	public:
		static constexpr int width = 800;
		static constexpr int height = 600;

		rotating_triangles_app();
		~rotating_triangles_app();

		rotating_triangles_app(const rotating_triangles_app&) = delete;
		rotating_triangles_app& operator=(const rotating_triangles_app&) = delete;

		void run();

	private:
		void load_game_objects();

		vk_window window{width, height, "Vulkan!"};
		vk_device device{window};
		vk_renderer renderer{window, device};

		vk_game_object::map game_objects;
	};
}
