#pragma once
#include <memory>

#include "editor/editor_ui.h"
#include "editor/renderer_in_ui.h"
#include "runtime/platform/timer.h"
#include "runtime/platform/window.h"
#include "runtime/renderer/renderer.h"
#include "runtime/runtime.h"

class Editor {
 public:
  Editor(Runtime& rt);
  ~Editor();
  void run();

 private:
  Runtime& runtime;
  std::unique_ptr<RendererInUI> renderer_in_ui{nullptr};
  std::unique_ptr<Window> main_window{nullptr};
  std::unique_ptr<EditorUI> ui{nullptr};

  Timer timer;
};