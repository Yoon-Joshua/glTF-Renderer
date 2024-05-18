#pragma once
#include "common/vk_common.h"
namespace vkb {
class Device;

class SemaphorePool {
 public:
  SemaphorePool(Device &device);

  SemaphorePool(const SemaphorePool &) = delete;

  SemaphorePool(SemaphorePool &&other) = delete;

  ~SemaphorePool();

  SemaphorePool &operator=(const SemaphorePool &) = delete;

  SemaphorePool &operator=(SemaphorePool &&) = delete;

  VkSemaphore request_semaphore();
  VkSemaphore request_semaphore_with_ownership();
  void release_owned_semaphore(VkSemaphore semaphore);

  void reset();

  uint32_t get_active_semaphore_count() const;

 private:
  Device &device;

  std::vector<VkSemaphore> semaphores;
  std::vector<VkSemaphore> released_semaphores;

  uint32_t active_semaphore_count{0};
};

}  // namespace vkb