#pragma once
#include "runtime/renderer/renderer.h"

class RendererInUI : public Renderer {
 public:
  RendererInUI(Infrastructure& infrastructure);
  ~RendererInUI();

  virtual void input_event(const InputEvent& input_event) override;

  void create_render_context() override;
  void prepare_render_context() override;

  VkSurfaceKHR request_surface_with_ownershipe();

  // 渲染结束后，将输出图像的布局转换为可采样的
  void draw(vkb::CommandBuffer& command_buffer,
            vkb::RenderTarget& render_target) override;

  void set_image_extent(uint32_t w, uint32_t h);

  void set_interactive_region(float, float, float, float);

  inline VkSurfaceFormatKHR get_surface_format() { return surface_format; }
  inline VkSurfaceCapabilitiesKHR get_surface_capabilities() {
    return capabilities;
  }
  inline uint32_t get_ui_queue_family() { return ui_queue_family; }
  inline VkQueue get_ui_queue() { return ui_queue; }
  inline VkDescriptorPool get_descriptor_pool() {
    return gui_resource.descriptor_pool;
  }
  inline VkPresentModeKHR get_present_mode() { return present_mode; }

  inline VkImageView get_image_view() { return frame.image_view; }
  inline VkSampler get_sampler() { return frame.sampler; }

 private:
  struct Frame {
    void prepare(vkb::Device&, VkImage, VkFormat);
    void cleanup();
    void recreate(VkImage, VkFormat);

    vkb::Device* device{nullptr};
    VkImageView image_view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
  } frame;

  struct GuiResource {
    VkDescriptorPool descriptor_pool{VK_NULL_HANDLE};
    void prepare(vkb::Device&);
    void cleanup(vkb::Device&);
  } gui_resource;

  uint32_t ui_queue_family = 0;
  VkQueue ui_queue{VK_NULL_HANDLE};
  VkSurfaceFormatKHR surface_format{};
  VkPresentModeKHR present_mode{};
  VkSurfaceCapabilitiesKHR capabilities{};

  /// @name Information of interactive region
  /// @brief Including the position of the upper left corner and the size of
  /// this rectangular region
  ///{
  float region_width = 0;
  float region_height = 0;
  float region_pos_x = 0;
  float region_pos_y = 0;
  ///}
};