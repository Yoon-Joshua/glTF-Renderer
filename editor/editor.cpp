#include "editor/editor.h"

#include "config/global_config.h"

extern struct GlobalConfig global_config;

Editor::Editor(Runtime& rt) : runtime(rt) {
  // For ImGui, load vulkan functions
  Infrastructure& infrastructure = runtime.get_infrastructure();
  VkInstance instance = infrastructure.get_instance().get_handle();
  ImGui_ImplVulkan_LoadFunctions(
      [](const char* function_name, void* vulkan_instance) {
        return vkGetInstanceProcAddr(
            *(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
      },
      &(instance));

  // create the main window
  Platform& platform = rt.get_platform();
  Window::Properties properties;
  main_window = std::make_unique<Window>(&platform, properties);

  // create renderer to draw the image in the UI.
  renderer_in_ui = std::make_unique<RendererInUI>(infrastructure);
  runtime.get_platform().register_app(renderer_in_ui.get());
  ApplicationOptions options{false, main_window.get()};
  renderer_in_ui->prepare(options);

  // create UI
  ui = std::make_unique<EditorUI>(*renderer_in_ui, *main_window);
  ui->prepare();
}

Editor::~Editor() {
  ui.reset();
  renderer_in_ui->finish();
  renderer_in_ui.reset();
  main_window.reset();
}

void Editor::run() {
  renderer_in_ui->load_assets(global_config.scene);
  renderer_in_ui->create_rendering_pipeline();

  while (!main_window->should_close()) {
    auto delta_time = 0.01 * static_cast<float>(timer.tick<Timer::Seconds>());
    main_window->process_events();
    renderer_in_ui->update(delta_time);
    ui->draw();
  }
}