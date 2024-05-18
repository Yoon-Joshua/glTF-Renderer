#pragma once

#include "runtime/platform/input_events.h"
#include "runtime/platform/window.h"

struct ApplicationOptions {
  bool benchmark_enabled{false};
  Window *window{nullptr};
};

class Application {
 public:
  /// @brief Handles input events of the window
  /// @param input_event The input event object
  virtual void input_event(const InputEvent &input_event);

 protected:
  float fps{0.0f};
  float frame_time{0.0f};  // In ms
  uint32_t frame_count{0};
  uint32_t last_frame_count{0};
  bool lock_simulation_speed{false};
  Window *window{nullptr};
};