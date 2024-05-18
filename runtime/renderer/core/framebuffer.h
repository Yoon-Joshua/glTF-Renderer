#pragma once
#include "runtime/renderer/common/helpers.h"
#include "runtime/renderer/common/vk_common.h"
#include "runtime/renderer/core/render_pass.h"
#include "runtime/renderer/rendering/render_target.h"
namespace vkb {
class Device;
class Framebuffer {
 public:
  Framebuffer(Device &device, const RenderTarget &render_target,
              const RenderPass &render_pass);

  Framebuffer(const Framebuffer &) = delete;

  Framebuffer(Framebuffer &&other);

  ~Framebuffer();

  Framebuffer &operator=(const Framebuffer &) = delete;

  Framebuffer &operator=(Framebuffer &&) = delete;

  VkFramebuffer get_handle() const;

  const VkExtent2D &get_extent() const;

 private:
  Device &device;

  VkFramebuffer handle{VK_NULL_HANDLE};

  VkExtent2D extent{};
};
}  // namespace vkb