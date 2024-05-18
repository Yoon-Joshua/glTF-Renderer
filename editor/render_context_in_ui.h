#pragma once
#include "runtime/renderer/rendering/render_context.h"
namespace vkb {
class RenderContextInUI : public RenderContext {
 public:
  RenderContextInUI(Device& device, const Window& window, uint32_t w,
                    uint32_t h);

  void prepare(size_t thread_count = 1,
               RenderTarget::CreateFunc create_render_target_func =
                   RenderTarget::DEFAULT_CREATE_FUNC);

  void recreate();

  inline VkImage get_canvas() { return canvas_image; }

  void set_canvas_extent(uint32_t, uint32_t);

 private:
  uint32_t canvas_width;
  uint32_t canvas_height;
  VkImage canvas_image{VK_NULL_HANDLE};
};
}  // namespace vkb