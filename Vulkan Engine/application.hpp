#pragma once

#include "vk_device.hpp"
#include "vk_game_object.hpp"
#include "vk_renderer.hpp"
#include "vk_window.hpp"

namespace vk_engine
{
	class application
	{
	public:
		static constexpr int width = 800;
		static constexpr int height = 600;

		application();
		~application();

		application(const application&) = delete;
		application& operator=(const application&) = delete;

		void run();

	private:
		void load_game_objects();

		vk_window window{width, height, "Vulkan!"};
		vk_device device{window};
		vk_renderer renderer{window, device};
		
		std::vector<vk_game_object> game_objects;
	};
}
