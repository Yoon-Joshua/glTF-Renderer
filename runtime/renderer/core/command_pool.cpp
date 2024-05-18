#include "command_pool.h"

#include "runtime/renderer/common/error.h"
#include "runtime/renderer/core/device.h"

namespace vkb {
CommandPool::CommandPool(Device& d, uint32_t queue_family_index,
                         RenderFrame* render_frame, size_t thread_index,
                         CommandBuffer::ResetMode reset_mode)
    : device{d},
      render_frame{render_frame},
      thread_index{thread_index},
      reset_mode{reset_mode} {
  VkCommandPoolCreateFlags flags;
  switch (reset_mode) {
    case CommandBuffer::ResetMode::ResetIndividually:
    case CommandBuffer::ResetMode::AlwaysAllocate:
      flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      break;
    case CommandBuffer::ResetMode::ResetPool:
    default:
      flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      break;
  }

  VkCommandPoolCreateInfo create_info{
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};

  create_info.queueFamilyIndex = queue_family_index;
  create_info.flags = flags;

  auto result =
      vkCreateCommandPool(device.get_handle(), &create_info, nullptr, &handle);

  if (result != VK_SUCCESS) {
    throw VulkanException{result, "Failed to create command pool"};
  }
}

CommandPool::CommandPool(CommandPool&& other)
    : device{other.device},
      handle{other.handle},
      queue_family_index{other.queue_family_index},
      primary_command_buffers{std::move(other.primary_command_buffers)},
      active_primary_command_buffer_count{
          other.active_primary_command_buffer_count},
      secondary_command_buffers{std::move(other.secondary_command_buffers)},
      active_secondary_command_buffer_count{
          other.active_secondary_command_buffer_count},
      render_frame{other.render_frame},
      thread_index{other.thread_index},
      reset_mode{other.reset_mode} {
  other.handle = VK_NULL_HANDLE;
  other.queue_family_index = 0;
  other.active_primary_command_buffer_count = 0;
  other.active_secondary_command_buffer_count = 0;
}

CommandPool::~CommandPool() {
  primary_command_buffers.clear();
  secondary_command_buffers.clear();
  // Destroy command pool
  if (handle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device.get_handle(), handle, nullptr);
  }
}

Device& CommandPool::get_device() { return device; }

VkCommandPool CommandPool::get_handle() const { return handle; }

RenderFrame* CommandPool::get_render_frame() { return render_frame; }

size_t CommandPool::get_thread_index() const { return thread_index; }

VkResult CommandPool::reset_pool() {
  VkResult result = VK_SUCCESS;

  switch (reset_mode) {
    case CommandBuffer::ResetMode::ResetIndividually: {
      result = reset_command_buffers();

      break;
    }
    case CommandBuffer::ResetMode::ResetPool: {
      result = vkResetCommandPool(device.get_handle(), handle, 0);

      if (result != VK_SUCCESS) {
        return result;
      }

      result = reset_command_buffers();

      break;
    }
    case CommandBuffer::ResetMode::AlwaysAllocate: {
      primary_command_buffers.clear();
      active_primary_command_buffer_count = 0;

      secondary_command_buffers.clear();
      active_secondary_command_buffer_count = 0;

      break;
    }
    default:
      throw std::runtime_error("Unknown reset mode for command pools");
  }

  return result;
}

CommandBuffer& CommandPool::request_command_buffer(VkCommandBufferLevel level) {
  if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    if (active_primary_command_buffer_count < primary_command_buffers.size()) {
      return *primary_command_buffers[active_primary_command_buffer_count++];
    }

    primary_command_buffers.emplace_back(
        std::make_unique<CommandBuffer>(*this, level));

    active_primary_command_buffer_count++;

    return *primary_command_buffers.back();
  } else {
    if (active_secondary_command_buffer_count <
        secondary_command_buffers.size()) {
      return *secondary_command_buffers
          [active_secondary_command_buffer_count++];
    }

    secondary_command_buffers.emplace_back(
        std::make_unique<CommandBuffer>(*this, level));

    active_secondary_command_buffer_count++;

    return *secondary_command_buffers.back();
  }
}

CommandBuffer::ResetMode const CommandPool::get_reset_mode() const {
  return reset_mode;
}

VkResult CommandPool::reset_command_buffers() {
  VkResult result = VK_SUCCESS;

  for (auto& cmd_buf : primary_command_buffers) {
    result = cmd_buf->reset(reset_mode);

    if (result != VK_SUCCESS) {
      return result;
    }
  }

  active_primary_command_buffer_count = 0;

  for (auto& cmd_buf : secondary_command_buffers) {
    result = cmd_buf->reset(reset_mode);

    if (result != VK_SUCCESS) {
      return result;
    }
  }

  active_secondary_command_buffer_count = 0;

  return result;
}

}  // namespace vkb