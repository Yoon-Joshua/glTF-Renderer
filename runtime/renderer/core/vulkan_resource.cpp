#include "vulkan_resource.h"

#include "runtime/renderer/core/device.h"

#define TEMP

namespace vkb {
namespace core {
namespace detail {
void set_debug_name(const Device *device, VkObjectType object_type,
                    uint64_t handle, const char *debug_name) {
#ifndef TEMP
  if (!debug_name || *debug_name == '\0' || !device) {
    // Can't set name, or no point in setting an empty name
    return;
  }

  device->get_debug_utils().set_debug_name(device->get_handle(), object_type,
                                           handle, debug_name);
#endif
}
}  // namespace detail
}  // namespace core
}  // namespace vkb