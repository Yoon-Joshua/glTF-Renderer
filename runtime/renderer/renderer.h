#pragma once

#include <iostream>

#include "runtime/platform/application.h"
#include "runtime/platform/configuration.h"
#include "runtime/platform/window.h"
#include "runtime/renderer/core/debug.h"
#include "runtime/renderer/core/device.h"
#include "runtime/renderer/core/physical_device.h"
#include "runtime/renderer/debug_info.h"
#include "runtime/renderer/infrastructure.h"
#include "runtime/renderer/rendering/render_context.h"
#include "runtime/renderer/rendering/render_pipeline.h"
#include "runtime/renderer/rendering/render_target.h"
#include "runtime/renderer/scene_graph/components/perspective_camera.h"
#include "runtime/renderer/scene_graph/scene.h"
#include "runtime/renderer/stats/stats.h"

class Renderer : public Application {
  /******************************* Application *******************************/
 public:
  Renderer(Infrastructure &Infrastructure);
  virtual ~Renderer();
  virtual bool prepare(ApplicationOptions &options);
  virtual void update(float delta_time);
  virtual void input_event(const InputEvent &input_event) override;
  virtual void finish();
  vkb::DebugInfo &get_debug_info();

 private:
  // The debug info of the app
  vkb::DebugInfo debug_info{};

  /****************************** Vulkan Sample ******************************/
 public:
  VkSurfaceKHR get_surface();
  vkb::Device &get_device();
  vkb::RenderContext &get_render_context();
  sg::Scene &get_scene();
  bool has_scene();

 protected:
  std::unique_ptr<vkb::Device> device{nullptr};

  /// @brief Context used for rendering, it is responsible for managing the
  /// frames and their underlying images
  std::unique_ptr<vkb::RenderContext> render_context{nullptr};

  /// @brief Pipeline used for rendering, it should be set up by the concrete
  /// sample
  std::unique_ptr<vkb::RenderPipeline> render_pipeline{nullptr};

  /// @brief Holds all scene information
  std::unique_ptr<sg::Scene> scene{nullptr};

  std::unique_ptr<vkb::Stats> stats{nullptr};

  /// @brief Update scene
  /// @param delta_time
  void update_scene(float delta_time);

  /// @brief Update counter values
  /// @param delta_time
  void update_stats(float delta_time);

  /// @brief Prepares the render target and draws to it, calling draw_renderpass
  /// @param command_buffer The command buffer to record the commands to
  /// @param render_target The render target that is being drawn to
  virtual void draw(vkb::CommandBuffer &command_buffer,
                    vkb::RenderTarget &render_target);

  /// @brief Starts the render pass, executes the render pipeline, and then ends
  /// the render pass
  /// @param command_buffer The command buffer to record the commands to
  /// @param render_target The render target that is being drawn to
  virtual void draw_renderpass(vkb::CommandBuffer &command_buffer,
                               vkb::RenderTarget &render_target);

  /// @brief Get sample-specific device extensions.
  /// @return Map of device extensions and whether or not they are optional.
  /// Default is empty map.
  const std::unordered_map<const char *, bool> get_device_extensions();

  /// @brief Add a sample-specific device extension
  /// @param extension The extension name
  /// @param optional (Optional) Whether the extension is optional
  void add_device_extension(const char *extension, bool optional = false);

  /// @brief Request features from the gpu based on what is supported
  virtual void request_gpu_features(vkb::PhysicalDevice &gpu);

  /// @brief Override this to customise the creation of the render_context
  virtual void create_render_context();

  /// @brief Override this to customise the creation of the swapchain and
  /// render_context
  virtual void prepare_render_context();

  /// @brief The Vulkan surface
  VkSurfaceKHR surface{VK_NULL_HANDLE};

  /// @brief The configuration of the sample
  Configuration configuration{};

  /// @brief A helper to create a render context
  void create_render_context(
      const std::vector<VkSurfaceFormatKHR> &surface_formats);

 private:
  /** @brief Set of device extensions to be enabled for this example and whether
   * they are optional (must be set in the derived constructor) */
  std::unordered_map<const char *, bool> device_extensions;

  /** @brief Whether or not we want a high priority graphics queue. */
  bool high_priority_graphics_queue{false};

  /********************************* XR.Y *********************************/
 public:
  void load_assets(std::string);
  
  /// @brief Deferred Pass
  void create_rendering_pipeline();

  sg::PerspectiveCamera *camera{};

 protected:
  std::unique_ptr<vkb::RenderTarget> create_render_target(
      vkb::core::Image &&swapchain_image);
  Infrastructure &infrastructure;
  bool headless{false};
};