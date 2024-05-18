#include "runtime/platform/platform.h"

#include <memory>

std::string Platform::external_storage_directory = "";
std::string Platform::temp_directory = "";

Platform::Platform() {}

const std::string &Platform::get_external_storage_directory() {
  return external_storage_directory;
}

const std::string &Platform::get_temp_directory() { return temp_directory; }

void Platform::input_event(const InputEvent &input_event) {
  if (process_input_events && active_app) {
    active_app->input_event(input_event);
  }

  // if (input_event.get_source() == EventSource::Keyboard) {
  //   const auto &key_event = static_cast<const KeyInputEvent &>(input_event);

  //   if (key_event.get_code() == KeyCode::Back ||
  //       key_event.get_code() == KeyCode::Escape) {
  //     close();
  //   }
  // }
}

void Platform::set_focus(bool _focused) { focused = _focused; }

/******************************* XR.Y *******************************/
void Platform::register_app(Application *app) { this->active_app = app; }

bool Platform::is_focused() { return focused; }
/******************************* XR.Y *******************************/