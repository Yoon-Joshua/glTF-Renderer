#include "preview_widget.h"

#include <imgui_impl_vulkan.h>

#include <iostream>

#include "runtime/platform/input_events.h"

PreviewWidget::PreviewWidget(RendererInUI& r) : renderer(r) {}

void PreviewWidget::set_title(std::string _title) { title = _title; }

void PreviewWidget::show() {
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Begin(title.c_str(), NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse);
  size = ImGui::GetWindowSize();
  pos = ImGui::GetWindowPos();
  renderer.set_interactive_region(pos.x, pos.y, size.x, size.y);
  const VkExtent2D extent = renderer.get_render_context()
                                .get_render_frames()[0]
                                ->get_render_target()
                                .get_extent();

  ImGui::Image((ImTextureID)ds, ImVec2(extent.width, extent.height));
  //ImGui::SetWindowSize(ImVec2{extent.width, extent.height});
  ImGui::End();
}

void PreviewWidget::add_texture(VkSampler sampler, VkImageView image_view) {
  ds = ImGui_ImplVulkan_AddTexture(sampler, image_view,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}