#pragma once
#include <imgui.h>
#include <Volk/volk.h>

#include <string>

#include "editor/renderer_in_ui.h"

class PreviewWidget {
 public:
  PreviewWidget(RendererInUI&);
  void set_title(std::string);

  void show();
  void add_texture(VkSampler, VkImageView);

 private:
  std::string title = "Preview Widget";
  VkDescriptorSet ds{VK_NULL_HANDLE};
  RendererInUI& renderer;

  ImVec2 size{0, 0};
  ImVec2 pos{0, 0};
};