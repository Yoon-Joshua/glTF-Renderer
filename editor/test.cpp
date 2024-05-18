#include "test.h"

#include "config/global_config.h"
#include "runtime/renderer/renderer.h"

extern struct GlobalConfig global_config;

Test::Test(Runtime& rt) : runtime(rt) {
  // For ImGui, load vulkan functions
  Infrastructure& infrastructure = runtime.get_infrastructure();
  VkInstance instance = infrastructure.get_instance().get_handle();

  // create the main window
  Window::Properties properties;
  Platform& platform = runtime.get_platform();
  main_window = std::make_unique<Window>(&platform, properties);

  // create renderer to draw the image in the UI.
  renderer = std::make_unique<Renderer>(infrastructure);
  runtime.get_platform().register_app(renderer.get());

  ApplicationOptions options{false, main_window.get()};
  renderer->prepare(options);
}

Test::~Test() {
  if (renderer) {
    renderer->finish();
  }
  renderer.reset();
  main_window.reset();
}

void Test::run() {
  renderer->load_assets(global_config.scene);
  renderer->create_rendering_pipeline();

  while (!main_window->should_close()) {
    auto delta_time = 0.0001 * static_cast<float>(timer.tick<Timer::Seconds>());

    auto platform = reinterpret_cast<Platform*>(
        glfwGetWindowUserPointer(main_window->get_handle()));

    if (platform->is_focused()) {
      renderer->update(delta_time);
    }
    main_window->process_events();
  }
}