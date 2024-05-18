#include "Volk/volk.h"
namespace vkb {
namespace initializers {

inline VkCommandBufferAllocateInfo command_buffer_allocate_info(
    VkCommandPool command_pool, VkCommandBufferLevel level,
    uint32_t buffer_count) {
  VkCommandBufferAllocateInfo command_buffer_allocate_info{};
  command_buffer_allocate_info.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_allocate_info.commandPool = command_pool;
  command_buffer_allocate_info.level = level;
  command_buffer_allocate_info.commandBufferCount = buffer_count;
  return command_buffer_allocate_info;
}

inline VkSemaphoreCreateInfo semaphore_create_info() {
  VkSemaphoreCreateInfo semaphore_create_info{};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  return semaphore_create_info;
}

inline VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0) {
  VkFenceCreateInfo fence_create_info{};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.flags = flags;
  return fence_create_info;
}

inline VkSubmitInfo submit_info() {
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  return submit_info;
}

}  // namespace initializers
}  // namespace vkb