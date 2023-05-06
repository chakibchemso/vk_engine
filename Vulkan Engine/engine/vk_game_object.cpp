#include "vk_game_object.hpp"

namespace vk_engine
{
	glm::mat4 transform_component::mat4() const
	{
		//transform = T * Ry * Rx * Rz * S
		auto transform = translate(glm::mat4(1.f), translation);
		transform = rotate(transform, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
		transform = rotate(transform, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
		transform = rotate(transform, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
		transform = glm::scale(transform, scale);
		return transform;
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
