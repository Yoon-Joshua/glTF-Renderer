#pragma once
#include <Volk/volk.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Platform;

class Window {
 public:
  struct Extent {
    uint32_t width;
    uint32_t height;
  };

  enum class Vsync { OFF, ON, Default };

  struct Properties {
    std::string title = "";
    bool resizable = true;
    Vsync vsync = Vsync::Default;
    Extent extent = {1280, 720};
  };

  static void initialize();
  static void terminate();
  static std::vector<const char*> get_required_instance_extensions();

  Window(Platform*, Window::Properties&);

  /// @brief Attempt to resize the window - not guaranteed to change
  /// @param extent The preferred window extent
  /// @return Extent The new window extent
  Extent resize(const Extent& extent);
  bool should_close();
  void process_events();
  void close();
  GLFWwindow* get_handle() const;
  virtual VkSurfaceKHR create_surface(VkInstance);
  const Extent& get_extent() const;
  const Properties& get_properties() const;

  /// @brief Get the display present info for the window if needed
  /// @param info Filled in when the method returns true
  /// @param src_width The width of the surface being presented
  /// @param src_height The height of the surface being presented
  /// @return true if the present info was filled in and should be used
  /// @return false if the extra present info should not be used. info is left
  /// untouched.
  virtual bool get_display_present_info(VkDisplayPresentInfoKHR* info,
                                        uint32_t src_width,
                                        uint32_t src_height) const;

 private:
  GLFWwindow* handle{nullptr};
  Properties properties;
};