#include "input_controller.hpp"

#include <limits>

using vk_engine::input_controller;

using namespace glm;

void input_controller::move_in_plane_xz(GLFWwindow* window, const float delta_time, vk_game_object& game_object) const
{
	vec3 rotation{0.f};

	if (glfwGetKey(window, keys.look_right) == GLFW_PRESS)
		rotation.y += 1.f;
	if (glfwGetKey(window, keys.look_left) == GLFW_PRESS)
		rotation.y -= 1.f;
	if (glfwGetKey(window, keys.look_up) == GLFW_PRESS)
		rotation.x += 1.f;
	if (glfwGetKey(window, keys.look_down) == GLFW_PRESS)
		rotation.x -= 1.f;

	if (dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
		game_object.transform.rotation += look_speed * delta_time * normalize(rotation);

	game_object.transform.rotation.x = clamp(game_object.transform.rotation.x, -90.f, 90.f);
	game_object.transform.rotation.y = mod(game_object.transform.rotation.y, 360.f);

	const float yaw = game_object.transform.rotation.y;
	const vec3 forward_dir{sin(yaw), 0.f, cos(yaw)};
	const vec3 right_dir{forward_dir.z, 0.f, -forward_dir.x};
	constexpr vec3 up_dir{0.f, -1.f, 0.f};

	vec3 move_dir{0.f};

	if (glfwGetKey(window, keys.move_forward) == GLFW_PRESS)
		move_dir += forward_dir;
	if (glfwGetKey(window, keys.move_backward) == GLFW_PRESS)
		move_dir -= forward_dir;
	if (glfwGetKey(window, keys.move_right) == GLFW_PRESS)
		move_dir += right_dir;
	if (glfwGetKey(window, keys.move_left) == GLFW_PRESS)
		move_dir -= right_dir;
	if (glfwGetKey(window, keys.move_up) == GLFW_PRESS)
		move_dir += up_dir;
	if (glfwGetKey(window, keys.move_down) == GLFW_PRESS)
		move_dir -= up_dir;

	if (dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon())
		game_object.transform.translation += move_speed * delta_time * normalize(move_dir);
}
