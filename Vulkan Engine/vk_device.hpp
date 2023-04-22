#pragma once

#include "vk_window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace vk_engine
{
    struct swap_chain_support_details
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
    };

    struct queue_family_indices
    {
        uint32_t graphics_family{};
        uint32_t present_family{};
        bool graphics_family_has_value = false;
        bool present_family_has_value = false;
        bool is_complete() const { return graphics_family_has_value && present_family_has_value; }
    };

    class vk_device
    {
    public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
        const bool enable_validation_layers = true;
#endif

        explicit vk_device(vk_window& window);
        ~vk_device();

        // Not copyable or movable
        vk_device(const vk_device&) = delete;
        void operator=(const vk_device&) = delete;
        vk_device(vk_device&&) = delete;
        vk_device& operator=(vk_device&&) = delete;

        VkCommandPool get_command_pool() const { return command_pool_; }
        VkDevice device() const { return device_; }
        VkSurfaceKHR surface() const { return surface_; }
        VkQueue graphics_queue() const { return graphics_queue_; }
        VkQueue present_queue() const { return present_queue_; }

        swap_chain_support_details get_swap_chain_support() const { return query_swap_chain_support(physical_device_); }
        uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags prop_flags) const;
        queue_family_indices find_physical_queue_families() const { return find_queue_families(physical_device_); }
        VkFormat find_supported_format(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        void create_buffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags prop_flags,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory) const;
        VkCommandBuffer begin_single_time_commands() const;
        void end_single_time_commands(VkCommandBuffer command_buffer) const;
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) const;
        void copy_buffer_to_image(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count) const;

        void create_image_with_info(
            const VkImageCreateInfo& image_info,
            VkMemoryPropertyFlags prop_flags,
            VkImage& image,
            VkDeviceMemory& image_memory) const;

        VkPhysicalDeviceProperties properties{};

    private:
        void create_instance();
        void setup_debug_messenger();
        void create_surface();
        void pick_physical_device();
        void create_logical_device();
        void create_command_pool();

        // helper functions
        bool is_device_suitable(VkPhysicalDevice device) const;
        std::vector<const char*> get_required_extensions() const;
        bool check_validation_layer_support() const;
        queue_family_indices find_queue_families(VkPhysicalDevice device) const;
        static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        void has_gflw_required_instance_extensions() const;
        bool check_device_extension_support(VkPhysicalDevice device) const;
        swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device) const;

        VkInstance instance_{};
        VkDebugUtilsMessengerEXT debug_messenger_{};
        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
        vk_window& window_;
        VkCommandPool command_pool_{};

        VkDevice device_{};
        VkSurfaceKHR surface_{};
        VkQueue graphics_queue_{};
        VkQueue present_queue_{};

        const std::vector<const char*> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
} // namespace vk
