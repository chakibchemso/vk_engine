#pragma once
#include <glm/glm.hpp>

namespace vk_engine
{
	class vk_camera
	{
	public:
		void set_orthographic_projection(float left, float right, float bottom, float top, float near, float far);
		void set_perspective_projection(float fov, float aspect, float near, float far);
		const glm::mat4& get_projection_matrix() const;

	private:
		glm::mat4 projection_matrix{1.f};
	};
}
