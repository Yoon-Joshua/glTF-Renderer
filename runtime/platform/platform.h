#pragma once

#include <memory>
#include <string>

#include "runtime/platform/application.h"
#include "runtime/platform/input_events.h"
#include "runtime/platform/plugins/plugin.h"
#include "runtime/platform/timer.h"

enum class ExitCode {
  Success = 0, /* App executed as expected */
  Help,        /* App should show help */
  Close,       /* App has been requested to close at initialization */
  FatalError   /* App encountered an unexpected error */
};

class Platform {
 public:
  Platform();

  virtual ~Platform() = default;

  /// @brief Returns the working directory of the application set by the
  /// platform
  /// @returns The path to the working directory
  static const std::string &get_external_storage_directory();

  /// @brief Returns the suitable directory for temporary files from the
  /// environment variables set in the system
  /// @returns The path to the temp folder on the system
  static const std::string &get_temp_directory();

  virtual void input_event(const InputEvent &input_event);

  void set_focus(bool focused);

 protected:
  Application *active_app{nullptr};

  /* App should continue processing input events */
  bool process_input_events{true};

  /* App is currently in focus at an operating system level */
  bool focused{true};

 private:
  /// static so can be references from vkb::fs
  static std::string external_storage_directory;

  /// static so can be references from vkb::fs
  static std::string temp_directory;

  /******************************* XR.Y *******************************/
 public:
  /// @brief Register a application, i.e. a renderer, with the platform, so that
  /// the application will be controlled by mouse and keyboard input.
  void register_app(Application *app);

  bool is_focused();
  /******************************* XR.Y *******************************/
};