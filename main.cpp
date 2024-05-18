#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "config/global_config.h"
#include "config/shader_macro_config.h"
#include "editor/editor.h"
#include "editor/test.h"
#include "runtime/renderer/renderer.h"
#include "runtime/runtime.h"

using json = nlohmann::json;

struct GlobalConfig global_config;
struct ShaderMacroConfig shader_macro_config;

int main() {
  std::ifstream config_file("../config/global_config.json");
  json data = json::parse(config_file);
  config_file.close();

  global_config.mode = data.at("mode") == "gui" ? GUI : FULL_WINDOW;
  global_config.scene = data["scene"];
  auto translation = data["camera"]["translation"];
  auto rotation = data["camera"]["rotation"];
  global_config.camera_translation =
      glm::vec3(translation[0], translation[1], translation[2]);
  global_config.camera_rotation =
      glm::vec3(rotation[0], rotation[1], rotation[2]);
  global_config.delta_time=data["delta_time"];

  shader_macro_config.load_config();

  std::unique_ptr<Runtime> runtime = std::make_unique<Runtime>();
  if (global_config.mode == GUI) {
    std::unique_ptr<Editor> editor = std::make_unique<Editor>(*runtime);
    editor->run();
    editor.reset();
  } else {
    std::unique_ptr<Test> test = std::make_unique<Test>(*runtime);
    test->run();
    test.reset();
  }
  runtime.reset();

  return 0;
}