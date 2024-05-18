#include "sampler.h"

#include "device.h"

namespace vkb {
namespace core {
Sampler::Sampler(Device const &d, const VkSamplerCreateInfo &info)
    : VulkanResource{VK_NULL_HANDLE, &d} {
  VK_CHECK(vkCreateSampler(device->get_handle(), &info, nullptr, &handle));
}

Sampler::Sampler(Sampler &&other) : VulkanResource{std::move(other)} {}

Sampler::~Sampler() {
  if (handle != VK_NULL_HANDLE) {
    vkDestroySampler(device->get_handle(), handle, nullptr);
  }
}

}  // namespace core
}  // namespace vkb
