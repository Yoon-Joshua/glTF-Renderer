#pragma once
#include <string>

#include "runtime/renderer/common/glm_common.h"

enum Mode { GUI, FULL_WINDOW };

struct GlobalConfig {
  enum Mode mode;
  std::string scene;
  glm::vec3 camera_translation;
  glm::vec3 camera_rotation;
  float delta_time;
};