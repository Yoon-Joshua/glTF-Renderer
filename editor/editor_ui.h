#include "editor/renderer_in_ui.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "runtime/renderer/renderer.h"
#include "editor/preview_widget.h"
#include "editor/scene_widget.h"

class EditorUI {
 public:
  EditorUI(RendererInUI& renderer, Window& window);
  ~EditorUI();
  void prepare();
  void draw();

 private:
  void frame_render(ImDrawData* draw_data);
  void frame_present();
  RendererInUI& renderer;
  Window& window;
  ImGui_ImplVulkanH_Window wd;
  bool swapchain_should_rebuild = false;

  PreviewWidget preview_widget;
  SceneWidget scene_widget;
};