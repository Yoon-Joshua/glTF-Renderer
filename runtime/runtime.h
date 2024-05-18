#pragma once
#include <memory>

#include "runtime/platform/platform.h"
#include "runtime/renderer/infrastructure.h"
#include "runtime/renderer/renderer.h"

class Runtime {
 public:
  Runtime();
  ~Runtime();

  Platform& get_platform();
  Renderer& get_renderer();
  Infrastructure& get_infrastructure();

 private:
  std::unique_ptr<Platform> platform{nullptr};
  // std::unique_ptr<Renderer> renderer{nullptr};
  std::unique_ptr<Infrastructure> infrastructure{nullptr};
};