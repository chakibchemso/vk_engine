#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "vk_model.hpp"

namespace vk_engine
{
	struct transform2d_component
	{
		glm::vec2 translation{};
		glm::vec2 scale{1.f, 1.f};
		float rotation;

		glm::mat2 mat2();
	};

	struct rigid_body2d_component
	{
		glm::vec2 velocity;
		float mass{1.0f};
	};

	class vk_game_object
	{
	public:
		vk_game_object(const vk_game_object&) = delete;
		vk_game_object(vk_game_object&&) = default;
		vk_game_object& operator=(const vk_game_object&) = delete;
		vk_game_object& operator=(vk_game_object&&) = default;

		using id_t = unsigned int;

		static vk_game_object create_game_object();

		std::shared_ptr<vk_model> model{};
		glm::vec3 color{};
		transform2d_component transform2d{};
		rigid_body2d_component rigid_body2d{};

	private:
		explicit vk_game_object(id_t object_id);

		id_t id;
	};
}
