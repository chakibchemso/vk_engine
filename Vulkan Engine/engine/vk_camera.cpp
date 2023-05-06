#include "vk_camera.hpp"

void vk_engine::vk_camera::set_orthographic_projection(
	const float left, const float right,
	const float bottom, const float top,
	const float near, const float far)
{
	projection_matrix = glm::mat4{1.0f};
	projection_matrix[0][0] = 2.f / (right - left);
	projection_matrix[1][1] = 2.f / (bottom - top);
	projection_matrix[2][2] = 1.f / (far - near);
	projection_matrix[3][0] = -(right + left) / (right - left);
	projection_matrix[3][1] = -(bottom + top) / (bottom - top);
	projection_matrix[3][2] = -near / (far - near);
}

void vk_engine::vk_camera::set_perspective_projection(
	const float fov, const float aspect,
	const float near, const float far)
{
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	const float tan_half_fov = tan(fov / 2.f);
	projection_matrix = glm::mat4{0.0f};
	projection_matrix[0][0] = 1.f / (aspect * tan_half_fov);
	projection_matrix[1][1] = 1.f / (tan_half_fov);
	projection_matrix[2][2] = far / (far - near);
	projection_matrix[2][3] = 1.f;
	projection_matrix[3][2] = -(far * near) / (far - near);
}

const glm::mat4& vk_engine::vk_camera::get_projection_matrix() const
{
	return projection_matrix;
}
