#include "runtime/renderer/core/physical_device.h"

#include "runtime/renderer/common/error.h"
#include "runtime/renderer/common/logging.h"
namespace vkb {
    PhysicalDevice::PhysicalDevice(Instance& instance,
        VkPhysicalDevice physical_device)
        : instance(instance), handle(physical_device) {
        vkGetPhysicalDeviceFeatures(physical_device, &features);
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

        LOGI("Found GPU: %s\n", properties.deviceName);

        uint32_t queue_family_properties_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            physical_device, &queue_family_properties_count, nullptr);
        queue_family_properties =
            std::vector<VkQueueFamilyProperties>(queue_family_properties_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
            &queue_family_properties_count,
            queue_family_properties.data());
    }

    Instance& PhysicalDevice::get_instance() const {
        return instance;
    }

    VkBool32 PhysicalDevice::is_present_supported(
        VkSurfaceKHR surface, uint32_t queue_family_index) const {
        VkBool32 present_supported{ VK_FALSE };
        if (surface != VK_NULL_HANDLE) {
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(handle, queue_family_index,
                surface, &present_supported));
        }
        return present_supported;
    }

    VkPhysicalDevice PhysicalDevice::get_handle() const {
        return handle;
    }

    const VkPhysicalDeviceFeatures& PhysicalDevice::get_features() const {
        return features;
    }

    const VkPhysicalDeviceProperties& PhysicalDevice::get_properties() const {
        return properties;
    }

    const VkPhysicalDeviceMemoryProperties& PhysicalDevice::get_memory_properties() const {
        return memory_properties;
    }

    const std::vector<VkQueueFamilyProperties>&
        PhysicalDevice::get_queue_family_properties() const {
        return queue_family_properties;
    }

    const VkPhysicalDeviceFeatures PhysicalDevice::get_requested_features() const {
        return requested_features;
    }

    VkPhysicalDeviceFeatures& PhysicalDevice::get_mutable_requested_features() {
        return requested_features;
    }

    void* PhysicalDevice::get_extension_feature_chain() const {
        return last_requested_extension_feature;
    }
}