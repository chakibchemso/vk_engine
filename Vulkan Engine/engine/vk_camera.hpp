#pragma once
#include <glm/glm.hpp>

namespace vk_engine
{
	class vk_camera
	{
	public:
		void set_orthographic_projection(float left, float right, float bottom, float top, float near, float far);
		void set_perspective_projection(float fov, float aspect, float near, float far);
		void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
		void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
		void set_view_yxz(glm::vec3 position, glm::vec3 rotation);
		const glm::mat4& get_projection() const;
		const glm::mat4& get_view() const;

	private:
		glm::mat4 projection_matrix{1.f};
		glm::mat4 view_matrix{1.f};
	};
}
