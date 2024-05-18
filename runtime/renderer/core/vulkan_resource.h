#pragma once
#include <Volk/volk.h>

#include <cassert>
#include <string>

namespace vkb {
class Device;
namespace core {

namespace detail {
void set_debug_name(const Device* device, VkObjectType object_type,
                    uint64_t handle, const char* debug_name);
}

/// Inherit this for any Vulkan object with a handle of type `THandle`.
///
/// This allows the derived class to store a Vulkan handle, and also a pointer
/// to the parent Device. It also allow for adding debug data to any Vulkan
/// object.
template <typename THandle, VkObjectType OBJECT_TYPE, typename Device = Device>
class VulkanResource {
 public:
  VulkanResource(THandle handle = VK_NULL_HANDLE, Device* device = nullptr)
      : handle(handle), device(device) {}

  VulkanResource(const VulkanResource&) = delete;
  VulkanResource& operator=(const VulkanResource&) = delete;

  VulkanResource(VulkanResource&& other)
      : handle{other.handle}, device{other.device} {
    set_debug_name(other.debug_name);
    other.handle = VK_NULL_HANDLE;
  }

  virtual ~VulkanResource() = default;

  inline Device& get_device() const {
    assert(device && "Device handle not set");
    return *device;
  }

  inline const THandle& get_handle() const { return handle; }

  inline const uint64_t get_handle_u64() const {
    // See https://github.com/KhronosGroup/Vulkan-Docs/issues/368 .
    // Dispatchable and non-dispatchable handle types are *not* necessarily
    // binary-compatible! Non-dispatchable handles _might_ be only 32-bit long.
    // This is because, on 32-bit machines, they might be a typedef to a 32-bit
    // pointer.
    using UintHandle =
        typename std::conditional<sizeof(THandle) == sizeof(uint32_t), uint32_t,
                                  uint64_t>::type;

    return static_cast<uint64_t>(reinterpret_cast<UintHandle>(handle));
  }

  inline const std::string& get_debug_name() const { return debug_name; }

  inline void set_debug_name(const std::string& name) {
    debug_name = name;
    detail::set_debug_name(device, OBJECT_TYPE, get_handle_u64(),
                           debug_name.c_str());
  }

 protected:
  THandle handle;
  Device* device;
  std::string debug_name;
};
}  // namespace core
}  // namespace vkb