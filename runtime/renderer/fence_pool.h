#pragma once
#include "common/helpers.h"
namespace vkb {

class Device;

class FencePool {
 public:
  FencePool(Device &device);

  FencePool(const FencePool &) = delete;

  FencePool(FencePool &&other) = delete;

  ~FencePool();

  FencePool &operator=(const FencePool &) = delete;

  FencePool &operator=(FencePool &&) = delete;

  VkFence request_fence();

  VkResult wait(uint32_t timeout = std::numeric_limits<uint32_t>::max()) const;

  VkResult reset();

 private:
  Device &device;

  std::vector<VkFence> fences;

  uint32_t active_fence_count{0};
};

}  // namespace vkb