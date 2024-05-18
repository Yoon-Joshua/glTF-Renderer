#pragma once
#include <imgui.h>

#include <string>

#include "editor/renderer_in_ui.h"
class SceneWidget {
 public:
  SceneWidget(RendererInUI&);
  void show();

 private:
  const std::string title = "Scene Widget";
  RendererInUI& renderer;

  /// @brief 为所有组件赋予序号，以供Inspector来查阅。
  std::vector<sg::Component*> components;
  std::unordered_map<sg::Component*, int> c2i;
};