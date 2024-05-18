#include "runtime/runtime.h"

Runtime::Runtime() {
  Window::initialize();
  platform = std::make_unique<Platform>();
  infrastructure = std::make_unique<Infrastructure>();
  infrastructure->prepare();
}

Runtime::~Runtime() {
  //renderer.reset();
  infrastructure.reset();
  platform.reset();
}

Platform& Runtime::get_platform() { return *platform; }

//Renderer& Runtime::get_renderer() { return *renderer; }

Infrastructure& Runtime::get_infrastructure() { return *infrastructure; }
