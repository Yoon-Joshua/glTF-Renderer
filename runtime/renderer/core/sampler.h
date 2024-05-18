#pragma once
#include "runtime/renderer/core/vulkan_resource.h"
namespace vkb {

class Device;

namespace core {

class Sampler
    : public VulkanResource<VkSampler, VK_OBJECT_TYPE_SAMPLER, const Device> {
 public:
  /// @brief Creates a Vulkan Sampler
  /// @param d The device to use
  /// @param info Creation details
  Sampler(Device const &d, const VkSamplerCreateInfo &info);

  Sampler(const Sampler &) = delete;

  Sampler(Sampler &&sampler);

  ~Sampler();

  Sampler &operator=(const Sampler &) = delete;

  Sampler &operator=(Sampler &&) = delete;
};
}  // namespace core
}  // namespace vkb