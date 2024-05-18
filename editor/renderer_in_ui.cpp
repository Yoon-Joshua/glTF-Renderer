#include "editor/renderer_in_ui.h"

#include "editor/render_context_in_ui.h"
#include <imgui.h>
#define VK_NO_PROTOTYPES
#include <imgui_impl_vulkan.h>
#include "runtime/renderer/common/error.h"
#include "runtime/renderer/core/queue.h"

RendererInUI::RendererInUI(Infrastructure& infrastructure)
    : Renderer(infrastructure) {}

RendererInUI::~RendererInUI() {
  frame.cleanup();
  gui_resource.cleanup(get_device());
}

void RendererInUI::input_event(const InputEvent& input_event) {
  if (input_event.get_source() == EventSource::Keyboard) {
    const auto& key_event = static_cast<const KeyInputEvent&>(input_event);
    if (key_event.get_action() == KeyAction::Down ||
        key_event.get_action() == KeyAction::Repeat) {
    }
  } else if (input_event.get_source() == EventSource::Mouse) {
    const auto& mouse_button =
        static_cast<const MouseButtonInputEvent&>(input_event);

    float x = mouse_button.get_pos_x();
    float y = mouse_button.get_pos_y();

    if (mouse_button.get_action() == MouseAction::Down) {
      if (x < region_pos_x || x > region_pos_x + region_width ||
          y < region_pos_y || y > region_pos_y + region_height) {
        return;
      }
    }
  }
  Renderer::input_event(input_event);
}

void RendererInUI::create_render_context() {
  const vkb::PhysicalDevice& gpu = get_device().get_gpu();
  vkb::Instance& instance = gpu.get_instance();
  ui_queue_family = device->get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
  ui_queue = device->get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0).get_handle();

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(gpu.get_handle(), ui_queue_family,
                                       surface, &res);

  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {
      VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
      VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
  const VkColorSpaceKHR requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  surface_format = ImGui_ImplVulkanH_SelectSurfaceFormat(
      gpu.get_handle(), surface, requestSurfaceImageFormat,
      (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
      requestSurfaceColorSpace);

  // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                      VK_PRESENT_MODE_IMMEDIATE_KHR,
                                      VK_PRESENT_MODE_FIFO_KHR};
#else
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
  present_mode = ImGui_ImplVulkanH_SelectPresentMode(
      gpu.get_handle(), surface, &present_modes[0],
      IM_ARRAYSIZE(present_modes));
  printf("[vulkan] Selected PresentMode = %d\n", present_mode);

  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.get_handle(), surface,
                                                     &capabilities));

  render_context =
      std::make_unique<vkb::RenderContextInUI>(*device, *window, 1920, 1280);
}

void RendererInUI::prepare_render_context() {
  Renderer::prepare_render_context();

  frame.prepare(
      get_device(),
      dynamic_cast<vkb::RenderContextInUI*>(render_context.get())->get_canvas(),
      dynamic_cast<vkb::RenderContextInUI*>(render_context.get())
          ->get_format());
  gui_resource.prepare(get_device());
}

VkSurfaceKHR RendererInUI::request_surface_with_ownershipe() {
  VkSurfaceKHR s = this->surface;
  this->surface = VK_NULL_HANDLE;
  return s;
}

void RendererInUI::set_interactive_region(float pos_x, float pos_y, float width,
                                          float height) {
  region_pos_x = pos_x;
  region_pos_y = pos_y;
  region_width = width;
  region_height = height;
}

void RendererInUI::draw(vkb::CommandBuffer& command_buffer,
                        vkb::RenderTarget& render_target) {
  Renderer::draw(command_buffer, render_target);

  auto& views = render_target.get_views();

  vkb::ImageMemoryBarrier memory_barrier{};
  memory_barrier.old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  memory_barrier.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
  memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  command_buffer.image_memory_barrier(views[0], memory_barrier);
}

void RendererInUI::set_image_extent(uint32_t w, uint32_t h) {
  auto context = dynamic_cast<vkb::RenderContextInUI*>(render_context.get());
  context->set_canvas_extent(w, h);
  context->recreate();
  frame.recreate(context->get_canvas(), surface_format.format);
}

void RendererInUI::Frame::prepare(vkb::Device& device, VkImage image,
                                  VkFormat format) {
  this->device = &device;

  VkImageViewCreateInfo image_view_info{};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.image = image;
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.format = format;
  image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_info.subresourceRange.baseMipLevel = 0;
  image_view_info.subresourceRange.levelCount = 1;
  image_view_info.subresourceRange.baseArrayLayer = 0;
  image_view_info.subresourceRange.layerCount = 1;
  VK_CHECK(vkCreateImageView(device.get_handle(), &image_view_info, nullptr,
                             &image_view));

  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.addressModeU =
      VK_SAMPLER_ADDRESS_MODE_REPEAT;  // outside image bounds just use border
                                       // color
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.minLod = -1000;
  sampler_info.maxLod = 1000;
  sampler_info.maxAnisotropy = 1.0f;
  VK_CHECK(
      vkCreateSampler(device.get_handle(), &sampler_info, nullptr, &sampler));
}

void RendererInUI::Frame::cleanup() {
  if (sampler != VK_NULL_HANDLE)
    vkDestroySampler(device->get_handle(), sampler, nullptr);
  if (image_view != VK_NULL_HANDLE)
    vkDestroyImageView(device->get_handle(), image_view, nullptr);
}

void RendererInUI::Frame::recreate(VkImage image, VkFormat format) {
  vkDestroyImageView(device->get_handle(), image_view, nullptr);
  vkDestroySampler(device->get_handle(), sampler, nullptr);

  VkImageViewCreateInfo image_view_info{};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.image = image;
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.format = format;
  image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_info.subresourceRange.baseMipLevel = 0;
  image_view_info.subresourceRange.levelCount = 1;
  image_view_info.subresourceRange.baseArrayLayer = 0;
  image_view_info.subresourceRange.layerCount = 1;
  VK_CHECK(vkCreateImageView(device->get_handle(), &image_view_info, nullptr,
                             &image_view));

  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.addressModeU =
      VK_SAMPLER_ADDRESS_MODE_REPEAT;  // outside image bounds just use border
                                       // color
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.minLod = -1000;
  sampler_info.maxLod = 1000;
  sampler_info.maxAnisotropy = 1.0f;
  VK_CHECK(
      vkCreateSampler(device->get_handle(), &sampler_info, nullptr, &sampler));
}

void RendererInUI::GuiResource::prepare(vkb::Device& device) {
  std::array<VkDescriptorPoolSize, 2> pool_sizes;
  pool_sizes[0] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
  pool_sizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 2;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  VK_CHECK(vkCreateDescriptorPool(device.get_handle(), &pool_info, nullptr,
                                  &descriptor_pool));
}

void RendererInUI::GuiResource::cleanup(vkb::Device& device) {
  if (descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device.get_handle(), descriptor_pool, nullptr);
    descriptor_pool = VK_NULL_HANDLE;
  }
}