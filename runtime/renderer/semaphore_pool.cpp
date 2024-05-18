#include "semaphore_pool.h"

#include "core/device.h"

namespace vkb {

SemaphorePool::SemaphorePool(Device& device) : device(device) {}

SemaphorePool::~SemaphorePool() {
  reset();

  // Destroy all semaphores
  for (VkSemaphore semaphore : semaphores) {
    vkDestroySemaphore(device.get_handle(), semaphore, nullptr);
  }

  semaphores.clear();
}

VkSemaphore SemaphorePool::request_semaphore() {
  // Check if there is an available semaphore
  if (active_semaphore_count < semaphores.size()) {
    return semaphores[active_semaphore_count++];
  }

  VkSemaphore semaphore{VK_NULL_HANDLE};

  VkSemaphoreCreateInfo create_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  VkResult result =
      vkCreateSemaphore(device.get_handle(), &create_info, nullptr, &semaphore);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphore.");
  }

  semaphores.push_back(semaphore);

  active_semaphore_count++;

  return semaphore;
}

VkSemaphore SemaphorePool::request_semaphore_with_ownership() {
  // Check if there is an available semaphore, if so, just pilfer one.
  if (active_semaphore_count < semaphores.size()) {
    VkSemaphore semaphore = semaphores.back();
    semaphores.pop_back();
    return semaphore;
  }

  // Otherwise, we need to create one, and don't keep track of it, app will
  // release.
  VkSemaphore semaphore{VK_NULL_HANDLE};

  VkSemaphoreCreateInfo create_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  VkResult result =
      vkCreateSemaphore(device.get_handle(), &create_info, nullptr, &semaphore);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphore.");
  }

  return semaphore;
}

void SemaphorePool::release_owned_semaphore(VkSemaphore semaphore) {
  // We cannot reuse this semaphore until ::reset().
  released_semaphores.push_back(semaphore);
}

void SemaphorePool::reset() {
  active_semaphore_count = 0;

  // Now we can safely recycle the released semaphores.
  for (auto& sem : released_semaphores) {
    semaphores.push_back(sem);
  }

  released_semaphores.clear();
}
}  // namespace vkb