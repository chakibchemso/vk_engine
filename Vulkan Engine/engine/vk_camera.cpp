#include "vk_camera.hpp"

using vk_engine::vk_camera;

void vk_camera::set_orthographic_projection(
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

void vk_camera::set_perspective_projection(
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

void vk_camera::set_view_direction(const glm::vec3 position, const glm::vec3 direction, const glm::vec3 up)
{
	const glm::vec3 w{normalize(direction)};
	const glm::vec3 u{normalize(cross(w, up))};
	const glm::vec3 v{cross(w, u)};

	view_matrix = glm::mat4{1.f};
	view_matrix[0][0] = u.x;
	view_matrix[1][0] = u.y;
	view_matrix[2][0] = u.z;
	view_matrix[0][1] = v.x;
	view_matrix[1][1] = v.y;
	view_matrix[2][1] = v.z;
	view_matrix[0][2] = w.x;
	view_matrix[1][2] = w.y;
	view_matrix[2][2] = w.z;
	view_matrix[3][0] = -dot(u, position);
	view_matrix[3][1] = -dot(v, position);
	view_matrix[3][2] = -dot(w, position);
}

void vk_camera::set_view_target(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up)
{
	set_view_direction(position, target - position, up);
}

void vk_camera::set_view_yxz(const glm::vec3 position, const glm::vec3 rotation)
{
	const float c3 = cos(rotation.z);
	const float s3 = sin(rotation.z);
	const float c2 = cos(rotation.x);
	const float s2 = sin(rotation.x);
	const float c1 = cos(rotation.y);
	const float s1 = sin(rotation.y);
	const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
	const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
	const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
	view_matrix = glm::mat4{1.f};
	view_matrix[0][0] = u.x;
	view_matrix[1][0] = u.y;
	view_matrix[2][0] = u.z;
	view_matrix[0][1] = v.x;
	view_matrix[1][1] = v.y;
	view_matrix[2][1] = v.z;
	view_matrix[0][2] = w.x;
	view_matrix[1][2] = w.y;
	view_matrix[2][2] = w.z;
	view_matrix[3][0] = -dot(u, position);
	view_matrix[3][1] = -dot(v, position);
	view_matrix[3][2] = -dot(w, position);
}

const glm::mat4& vk_camera::get_projection() const
{
	return projection_matrix;
}

const glm::mat4& vk_camera::get_view() const
{
	return view_matrix;
}
