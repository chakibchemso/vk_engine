#pragma once

#include "../engine/vk_game_object.hpp"
#include "../renderer/vk_device.hpp"
#include "../renderer/vk_renderer.hpp"
#include "../renderer/vk_window.hpp"
#include "../simple_render_system/vk_descriptors.hpp"

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

		//order matters
		std::unique_ptr<vk_descriptor_pool> global_pool{};
		std::vector<vk_game_object> game_objects;
	};
}
