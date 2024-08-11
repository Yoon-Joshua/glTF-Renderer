#include "runtime/renderer/renderer.h"

#include <cstring>
#include <fstream>

#include "config/global_config.h"
#include "runtime/renderer/common/error.h"
#include "runtime/renderer/common/glm_common.h"
#include "runtime/renderer/common/utils.h"
#include "runtime/renderer/common/vk_common.h"
#include "runtime/renderer/core/buffer.h"
#include "runtime/renderer/gltf_loader.h"
#include "runtime/renderer/rendering/subpass/geometry_subpass.h"
#include "runtime/renderer/rendering/subpass/lighting_subpass.h"
#include "runtime/renderer/scene_graph/components/pbr_material.h"
#include "runtime/renderer/scene_graph/script.h"

extern struct GlobalConfig global_config;

Renderer::Renderer(Infrastructure& infrastructure)
    : infrastructure(infrastructure), headless(infrastructure.is_headless()) {}

Renderer::~Renderer() {
  if (device) {
    device->wait_idle();
  }

  // test_triangle.reset();
  scene.reset();

  stats.reset();

  render_context.reset();
  device.reset();

  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(infrastructure.get_instance().get_handle(), surface,
                        nullptr);
  }
}

bool Renderer::prepare(ApplicationOptions& options) {
  assert(options.window != nullptr && "Window must be valid");

  auto& _debug_info = get_debug_info();
  _debug_info.insert<vkb::field::MinMax, float>("fps", fps);
  _debug_info.insert<vkb::field::MinMax, float>("frame_time", frame_time);

  lock_simulation_speed = options.benchmark_enabled;
  window = options.window;

  // Getting a valid vulkan surface from the platform
  surface = window->create_surface(infrastructure.get_instance().get_handle());
  if (!surface) {
    throw std::runtime_error("Failed to create window surface.");
  }

  auto& gpu = infrastructure.get_instance().get_suitable_gpu(surface);
  gpu.set_high_priority_graphics_queue_enable(high_priority_graphics_queue);

  // Request to enable ASTC
  if (gpu.get_features().textureCompressionASTC_LDR) {
    gpu.get_mutable_requested_features().textureCompressionASTC_LDR = VK_TRUE;
  }

  // Request sample required GPU features
  request_gpu_features(gpu);

  // Creating vulkan device, specifying the swapchain extension always
  if (!headless || infrastructure.get_instance().is_enabled(
                       VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)) {
    add_device_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (infrastructure.get_instance_extensions().find(
            VK_KHR_DISPLAY_EXTENSION_NAME) !=
        infrastructure.get_instance_extensions().end()) {
      add_device_extension(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,
                           /*optional=*/true);
    }
  }

  std::unique_ptr<vkb::DebugUtils> debug_utils = nullptr;

#ifdef VKB_VULKAN_DEBUG
  {
    auto instance_extensions = infrastructure.get_instance_extensions();
    for (const auto& it : instance_extensions) {
      if (strcmp(it.first, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
        LOGI("Vulkan debug utils enabled ({})",
             VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        debug_utils = std::make_unique<vkb::DebugUtilsExtDebugUtils>();
        break;
      }
    }
  }
#endif

#ifdef VKB_VULKAN_DEBUG
  if (!debug_utils) {
    uint32_t device_extension_count;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(
        gpu.get_handle(), nullptr, &device_extension_count, nullptr));

    std::vector<VkExtensionProperties> available_device_extensions(
        device_extension_count);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(
        gpu.get_handle(), nullptr, &device_extension_count,
        available_device_extensions.data()));

    for (const auto& it : available_device_extensions) {
      if (strcmp(it.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
        LOGI("Vulkan debug utils enabled ({})",
             VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

        debug_utils = std::make_unique<vkb::DebugMarkerExtDebugUtils>();
        add_device_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        break;
      }
    }
  }

  if (!debug_utils) {
    LOGW(
        "Vulkan debug utils were requested, but no extension that provides "
        "them was found");
  }
#endif

  if (!debug_utils) {
    debug_utils = std::make_unique<vkb::DummyDebugUtils>();
  }

  if (!device) {
    device = std::make_unique<vkb::Device>(gpu, surface, std::move(debug_utils),
                                           get_device_extensions());
  }

  create_render_context();
  prepare_render_context();

  stats = std::make_unique<vkb::Stats>(*render_context);

  // Start the sample in the first GUI configuration
  configuration.reset();
  return true;
}

void Renderer::update(float delta_time) {
  update_scene(delta_time);

  auto& command_buffer = render_context->begin();

  // Collect the performance data for the sample graphs
  update_stats(delta_time);

  command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  stats->begin_sampling(command_buffer);

  draw(command_buffer, render_context->get_active_frame().get_render_target());

  stats->end_sampling(command_buffer);

  command_buffer.end();

  render_context->submit(command_buffer);
}

void Renderer::input_event(const InputEvent& input_event) {
  Application::input_event(input_event);

  bool gui_captures_event = false;

  // if (gui) {
  //   gui_captures_event = gui->input_event(input_event);
  // }

  if (!gui_captures_event) {
    if (scene && scene->has_component<sg::Script>()) {
      auto scripts = scene->get_components<sg::Script>();

      for (auto script : scripts) {
        script->input_event(input_event);
      }
    }
  }

  if (input_event.get_source() == EventSource::Keyboard) {
    const auto& key_event = static_cast<const KeyInputEvent&>(input_event);
    if (key_event.get_action() == KeyAction::Down &&
        (key_event.get_code() == KeyCode::PrintScreen ||
         key_event.get_code() == KeyCode::F12)) {
      // screenshot(*render_context, "screenshot-" + get_name());
      printf(
          "screenshot is requested, but I haven't implemented it. I'm "
          "Sorry.\n");
    }
  }
}

void Renderer::finish() {
  if (device) {
    device->wait_idle();
  }
}

vkb::DebugInfo& Renderer::get_debug_info() { return debug_info; }

VkSurfaceKHR Renderer::get_surface() { return surface; }

vkb::Device& Renderer::get_device() { return *device; }

vkb::RenderContext& Renderer::get_render_context() {
  assert(render_context && "Render context is not valid");
  return *render_context;
}

sg::Scene& Renderer::get_scene() {
  assert(scene && "Scene not loaded");
  return *scene;
}

bool Renderer::has_scene() { return scene != nullptr; }

void Renderer::update_scene(float delta_time) {
  if (scene) {
    // Update scripts
    if (scene->has_component<sg::Script>()) {
      auto scripts = scene->get_components<sg::Script>();

      for (auto script : scripts) {
        script->update(delta_time);
      }
    }

    // // Update animations
    // if (scene->has_component<sg::Animation>()) {
    //   auto animations = scene->get_components<sg::Animation>();

    //   for (auto animation : animations) {
    //     animation->update(delta_time);
    //   }
    // }
  }
}

void Renderer::update_stats(float delta_time) {}

void Renderer::draw(vkb::CommandBuffer& command_buffer,
                    vkb::RenderTarget& render_target) {
  auto& views = render_target.get_views();
  assert(1 < views.size());

  {
    // Image 0 is the swapchain
    vkb::ImageMemoryBarrier memory_barrier{};
    memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    memory_barrier.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memory_barrier.src_access_mask = 0;
    memory_barrier.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.src_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier.dst_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    command_buffer.image_memory_barrier(views[0], memory_barrier);

    // Skip 1 as it is handled later as a depth-stencil attachment
    for (size_t i = 2; i < views.size(); ++i) {
      command_buffer.image_memory_barrier(views[i], memory_barrier);
    }
  }

  {
    vkb::ImageMemoryBarrier memory_barrier{};
    memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    memory_barrier.new_layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    memory_barrier.src_access_mask = 0;
    memory_barrier.dst_access_mask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    command_buffer.image_memory_barrier(views[1], memory_barrier);
  }

  draw_renderpass(command_buffer, render_target);

  {
    vkb::ImageMemoryBarrier memory_barrier{};
    memory_barrier.old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memory_barrier.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.src_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    command_buffer.image_memory_barrier(views[0], memory_barrier);
  }
}

void Renderer::draw_renderpass(vkb::CommandBuffer& command_buffer,
                               vkb::RenderTarget& render_target) {
  auto& extent = render_target.get_extent();

  VkViewport viewport{};
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  command_buffer.set_viewport(0, {viewport});

  VkRect2D scissor{};
  scissor.extent = extent;
  command_buffer.set_scissor(0, {scissor});

  render_pipeline->draw(command_buffer, render_target);

  command_buffer.end_render_pass();
}

const std::unordered_map<const char*, bool> Renderer::get_device_extensions() {
  return device_extensions;
}

void Renderer::add_device_extension(const char* extension, bool optional) {
  device_extensions[extension] = optional;
}

void Renderer::request_gpu_features(vkb::PhysicalDevice& gpu) {
  // To be overridden
}

void Renderer::create_render_context() {
  auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{
      {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
  create_render_context(surface_priority_list);
}

void Renderer::prepare_render_context() {
  // render_context->prepare();

  // 针对two pass延迟渲染
  get_render_context().prepare(1, [this](vkb::core::Image&& swapchain_image) {
    return create_render_target(std::move(swapchain_image));
  });
}

void Renderer::create_render_context(
    const std::vector<VkSurfaceFormatKHR>& surface_priority_list) {
  VkPresentModeKHR present_mode =
      (window->get_properties().vsync == Window::Vsync::ON)
          ? VK_PRESENT_MODE_FIFO_KHR
          : VK_PRESENT_MODE_MAILBOX_KHR;
  std::vector<VkPresentModeKHR> present_mode_priority_list{
      VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_IMMEDIATE_KHR};
  render_context = std::make_unique<vkb::RenderContext>(
      get_device(), surface, *window, present_mode, present_mode_priority_list,
      surface_priority_list);
}

/********************************* XR.Y *********************************/
#define TEMP
void Renderer::load_assets(std::string path) {
  vkb::GLTFLoader loader{*device};
  scene = loader.read_scene_from_file(path);

  if (!scene) {
    LOGE("Cannot load scene: {}", path.c_str());
    throw std::runtime_error("Cannot load scene: " + path);
  }
}

void Renderer::create_rendering_pipeline() {
#define XRY
#ifndef XRY
  Geometry subpass sg::Node& camera_node = add_free_camera(
      *scene, "default_camera", get_render_context().get_surface_extent());
#else
  // 这个地方应该是rendertarget的extent，而不是surface的extent
  VkExtent2D target_extent =
      render_context->get_render_frames()[0]->get_render_target().get_extent();
  sg::Node& camera_node =
      add_free_camera(*scene, "default_camera", target_extent);
#endif
#undef XRY

  camera = dynamic_cast<sg::PerspectiveCamera*>(
      &camera_node.get_component<sg::Camera>());

  camera->get_node()->get_transform().set_rotation(
      glm::quat(global_config.camera_rotation));
  camera->get_node()->get_transform().set_translation(
      global_config.camera_translation);

  vkb::ShaderSource geometry_vs =
      vkb::ShaderSource{"deferred/geometry.vert"};
  vkb::ShaderSource geometry_fs =
      vkb::ShaderSource{"deferred/geometry.frag"};
  auto scene_subpass = std::make_unique<vkb::GeometrySubpass>(
      get_render_context(), std::move(geometry_vs), std::move(geometry_fs),
      *scene, *camera);

  // Outputs are depth, albedo, and normal, and metallic-roughness
  scene_subpass->set_output_attachments({1, 2, 3, 4});

  // Lighting subpass
  auto lighting_vs = vkb::ShaderSource{"deferred/lighting.vert"};
  auto lighting_fs = vkb::ShaderSource{"deferred/lighting.frag"};
  auto lighting_subpass = std::make_unique<vkb::LightingSubpass>(
      get_render_context(), std::move(lighting_vs), std::move(lighting_fs),
      *camera, *scene);

  // Inputs are depth, albedo, and normal from the geometry subpass
  lighting_subpass->set_input_attachments({1, 2, 3, 4});

  // Create subpasses pipeline
  std::vector<std::unique_ptr<vkb::Subpass>> subpasses{};
  subpasses.push_back(std::move(scene_subpass));
  subpasses.push_back(std::move(lighting_subpass));

  render_pipeline = std::make_unique<vkb::RenderPipeline>(std::move(subpasses));

  render_pipeline->set_load_store(
      vkb::gbuffer::get_clear_all_store_swapchain());

  render_pipeline->set_clear_value(vkb::gbuffer::get_clear_value());
}

std::unique_ptr<vkb::RenderTarget> Renderer::create_render_target(
    vkb::core::Image&& swapchain_image) {
  VkFormat albedo_format{VK_FORMAT_R8G8B8A8_UNORM};
  VkFormat normal_format{VK_FORMAT_A2B10G10R10_UNORM_PACK32};
  VkFormat metallic_roughness_format{VK_FORMAT_R8G8B8A8_UNORM};
  VkImageUsageFlags rt_usage_flags{VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT};

  auto& device = swapchain_image.get_device();
  auto& extent = swapchain_image.get_extent();

  // G-Buffer should fit 128-bit budget for buffer color storage
  // in order to enable subpasses merging by the driver
  // Light (swapchain_image) RGBA8_UNORM   (32-bit)
  // Albedo                  RGBA8_UNORM   (32-bit)
  // Normal                  RGB10A2_UNORM (32-bit)
  // TODO: Metallic-Roughness RGBA8_UNORM  (32-bit)

  vkb::core::Image depth_image{
      device, extent,
      vkb::get_suitable_depth_format(
          swapchain_image.get_device().get_gpu().get_handle()),
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | rt_usage_flags,
      VMA_MEMORY_USAGE_GPU_ONLY};

  vkb::core::Image albedo_image{
      device, extent, albedo_format,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | rt_usage_flags,
      VMA_MEMORY_USAGE_GPU_ONLY};

  vkb::core::Image normal_image{
      device, extent, normal_format,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | rt_usage_flags,
      VMA_MEMORY_USAGE_GPU_ONLY};

  vkb::core::Image metallic_roughness_image{
      device, extent, metallic_roughness_format,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | rt_usage_flags,
      VMA_MEMORY_USAGE_GPU_ONLY};

  std::vector<vkb::core::Image> images;

  // Attachment 0
  images.push_back(std::move(swapchain_image));

  // Attachment 1
  images.push_back(std::move(depth_image));

  // Attachment 2
  images.push_back(std::move(albedo_image));

  // Attachment 3
  images.push_back(std::move(normal_image));

  // Attachment 4
  images.push_back(std::move(metallic_roughness_image));

  return std::make_unique<vkb::RenderTarget>(std::move(images));
}
#undef TEMP