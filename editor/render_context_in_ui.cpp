#include "render_context_in_ui.h"
namespace vkb {
RenderContextInUI::RenderContextInUI(Device& device, const Window& window,
                                     uint32_t w, uint32_t h)
    : RenderContext(device, window), canvas_width(w), canvas_height(h) {}

void RenderContextInUI::prepare(
    size_t thread_count, RenderTarget::CreateFunc create_render_target_func) {
  Device& device = get_device();

  auto color_image = core::Image(
      device, VkExtent3D{canvas_width, canvas_height, 1},
      DEFAULT_VK_FORMAT,  // We can use any format here that we like
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
          VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);
  canvas_image = color_image.get_handle();

  auto render_target = create_render_target_func(std::move(color_image));
  frames.push_back(std::make_unique<RenderFrame>(
      device, std::move(render_target), thread_count));

  this->create_render_target_func = create_render_target_func;
  this->thread_count = thread_count;
  this->prepared = true;
}

void RenderContextInUI::recreate() {
  auto color_image = core::Image(
      device, VkExtent3D{canvas_width, canvas_height, 1},
      DEFAULT_VK_FORMAT,  // We can use any format here that we like
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);
  canvas_image = color_image.get_handle();

  auto render_target = create_render_target_func(std::move(color_image));
  frames.resize(0);
  frames.push_back(std::make_unique<RenderFrame>(
      device, std::move(render_target), thread_count));
}

void RenderContextInUI::set_canvas_extent(uint32_t w, uint32_t h) {
  canvas_width = w;
  canvas_height = h;
}

}  // namespace vkb