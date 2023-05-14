#pragma once
#include <memory>
#include <glm/ext.hpp>

#include "vk_model.hpp"

namespace vk_engine
{
	struct transform_component
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f};
		glm::vec3 rotation;

		glm::mat4 mat4() const;
		glm::mat3 normal_matrix() const;
	};

	struct rigid_body_component
	{
		glm::vec3 velocity;
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
		transform_component transform{};
		rigid_body_component rigid_body{};

	private:
		explicit vk_game_object(id_t object_id);

		id_t id;
	};
}
