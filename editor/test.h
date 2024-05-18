#pragma once

#include <memory>

#include "runtime/platform/timer.h"
#include "runtime/platform/window.h"
#include "runtime/renderer/renderer.h"
#include "runtime/runtime.h"

class Test {
 public:
  Test(Runtime& rt);
  ~Test();
  void run();

 private:
  Runtime& runtime;
  std::unique_ptr<Renderer> renderer{nullptr};
  std::unique_ptr<Window> main_window{nullptr};

  Timer timer;
};