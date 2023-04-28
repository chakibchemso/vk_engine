#include "vk_game_object.hpp"

namespace vk_engine
{
	glm::mat2 transform2d_component::mat2()
	{
		const float sin = glm::sin(rotation);
		const float cos = glm::cos(rotation);

		const glm::mat2 rotation_matrix{{cos, sin}, {-sin, cos}};
		const glm::mat2 scale_matrix{{scale.x, .0f}, {.0f, scale.y}};

		return rotation_matrix * scale_matrix;
	}

	vk_game_object vk_game_object::create_game_object()
	{
		static id_t current_id = 0;
		return vk_game_object(current_id++);
	}

	vk_game_object::vk_game_object(const id_t object_id) : id(object_id)
	{
	}
}
