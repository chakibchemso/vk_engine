#include "vk_game_object.hpp"

using vk_engine::vk_game_object;
using vk_engine::transform_component;

using namespace glm;

mat4 transform_component::mat4() const
{
	//transform = T * Ry * Rx * Rz * S
	auto transform = translate(glm::mat4(1.f), translation);
	transform = rotate(transform, radians(rotation.y), vec3(0.f, 1.f, 0.f));
	transform = rotate(transform, radians(rotation.x), vec3(1.f, 0.f, 0.f));
	transform = rotate(transform, radians(rotation.z), vec3(0.f, 0.f, 1.f));
	transform = glm::scale(transform, scale);
	return transform;
}

mat3 transform_component::normal_matrix() const
{
	const float c3 = cos(rotation.z);
	const float s3 = sin(rotation.z);
	const float c2 = cos(rotation.x);
	const float s2 = sin(rotation.x);
	const float c1 = cos(rotation.y);
	const float s1 = sin(rotation.y);
	const vec3 inv_scale = 1.0f / scale;

	return mat3{
		{
			inv_scale.x * (c1 * c3 + s1 * s2 * s3),
			inv_scale.x * (c2 * s3),
			inv_scale.x * (c1 * s2 * s3 - c3 * s1),
		},
		{
			inv_scale.y * (c3 * s1 * s2 - c1 * s3),
			inv_scale.y * (c2 * c3),
			inv_scale.y * (c1 * c3 * s2 + s1 * s3),
		},
		{
			inv_scale.z * (c2 * s1),
			inv_scale.z * (-s2),
			inv_scale.z * (c1 * c2),
		},
	};
}

vk_game_object vk_game_object::create_game_object()
{
	static id_t current_id = 0;
	return vk_game_object(current_id++);
}

vk_game_object::vk_game_object(const id_t object_id) : id(object_id)
{
}
