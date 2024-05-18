#include "render_pipeline.h"

namespace vkb {
RenderPipeline::RenderPipeline(
    std::vector<std::unique_ptr<Subpass>> &&subpasses_)
    : subpasses{std::move(subpasses_)} {
  prepare();

  // Default clear value
  clear_value[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clear_value[1].depthStencil = {0.0f, ~0U};
}

void RenderPipeline::prepare() {
  for (auto &subpass : subpasses) {
    subpass->prepare();
  }
}

void RenderPipeline::set_load_store(const std::vector<LoadStoreInfo> &ls) {
  load_store = ls;
}

void RenderPipeline::set_clear_value(const std::vector<VkClearValue> &cv) {
  clear_value = cv;
}

void RenderPipeline::draw(CommandBuffer &command_buffer,
                          RenderTarget &render_target,
                          VkSubpassContents contents) {
  assert(!subpasses.empty() &&
         "Render pipeline should contain at least one sub-pass");

  // Pad clear values if they're less than render target attachments
  while (clear_value.size() < render_target.get_attachments().size()) {
    clear_value.push_back({0.0f, 0.0f, 0.0f, 1.0f});
  }

  for (size_t i = 0; i < subpasses.size(); ++i) {
    active_subpass_index = i;

    auto &subpass = subpasses[i];

    subpass->update_render_target_attachments(render_target);

    if (i == 0) {
      command_buffer.begin_render_pass(render_target, load_store, clear_value,
                                       subpasses, contents);
    } else {
      command_buffer.next_subpass();
    }
#define TEMP
    if (subpass->get_debug_name().empty()) {
#ifndef TEMP
      subpass->set_debug_name(fmt::format("RP subpass #{}", i));
#endif
    }
#undef TEMP
    ScopedDebugLabel subpass_debug_label{command_buffer,
                                         subpass->get_debug_name().c_str()};

    subpass->draw(command_buffer);
  }

  active_subpass_index = 0;
}

}  // namespace vkb