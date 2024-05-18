#pragma once
#include <Volk/volk.h>

#include <cstdint>

#include "runtime/renderer/core/command_buffer.h"

namespace vkb {

class Device;
class RenderFrame;
class CommandPool {
 public:
  CommandPool(Device& device, uint32_t queue_family_index,
              RenderFrame* render_frame = nullptr, size_t thread_index = 0,
              CommandBuffer::ResetMode reset_mode =
                  CommandBuffer::ResetMode::ResetPool);
  CommandPool(const CommandPool&) = delete;

  CommandPool(CommandPool&& other);

  ~CommandPool();

  CommandPool& operator=(const CommandPool&) = delete;

  CommandPool& operator=(CommandPool&&) = delete;

  Device& get_device();

  uint32_t get_queue_family_index() const;

  VkCommandPool get_handle() const;

  RenderFrame* get_render_frame();

  size_t get_thread_index() const;

  VkResult reset_pool();

  CommandBuffer& request_command_buffer(
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

  const CommandBuffer::ResetMode get_reset_mode() const;

 private:
  Device& device;
  VkCommandPool handle{VK_NULL_HANDLE};
  RenderFrame* render_frame{nullptr};
  size_t thread_index{0};
  uint32_t queue_family_index{0};

  std::vector<std::unique_ptr<CommandBuffer>> primary_command_buffers;

  uint32_t active_primary_command_buffer_count{0};

  std::vector<std::unique_ptr<CommandBuffer>> secondary_command_buffers;

  uint32_t active_secondary_command_buffer_count{0};

  CommandBuffer::ResetMode reset_mode{CommandBuffer::ResetMode::ResetPool};

  VkResult reset_command_buffers();
};
}  // namespace vkb