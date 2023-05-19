#pragma once

#include "../engine/vk_camera.hpp"
#include "vulkan/vulkan.h"

namespace vk_engine
{
	struct vk_frame_info
	{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		vk_camera& camera;
	};
}
