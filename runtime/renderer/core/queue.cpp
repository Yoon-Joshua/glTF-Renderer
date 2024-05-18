#include "queue.h"

#include "runtime/renderer/core/device.h"

namespace vkb {
Queue::Queue(Device& device, uint32_t family_index,
             VkQueueFamilyProperties properties, VkBool32 can_present,
             uint32_t index)
    : device{device},
      family_index{family_index},
      index{index},
      can_present{can_present},
      properties{properties} {
  vkGetDeviceQueue(device.get_handle(), family_index, index, &handle);
}

VkQueue Queue::get_handle() const { return handle; }

uint32_t Queue::get_family_index() const { return family_index; }

const VkQueueFamilyProperties& Queue::get_properties() const {
  return properties;
}

VkBool32 Queue::support_present() const { return can_present; }

VkResult Queue::submit(const std::vector<VkSubmitInfo>& submit_infos,
                       VkFence fence) const {
  return vkQueueSubmit(handle, to_u32(submit_infos.size()), submit_infos.data(),
                       fence);
}

VkResult Queue::submit(const CommandBuffer& command_buffer,
                       VkFence fence) const {
  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer.get_handle();

  return submit({submit_info}, fence);
}

VkResult Queue::present(const VkPresentInfoKHR& present_info) const {
  if (!can_present) {
    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
  }
  return vkQueuePresentKHR(handle, &present_info);
}

VkResult Queue::wait_idle() const { return vkQueueWaitIdle(handle); }
}  // namespace vkb