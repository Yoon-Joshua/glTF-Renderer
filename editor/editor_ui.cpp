#include "editor_ui.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <stdexcept>

static void check_vk_result(VkResult err) {
  if (err == 0) return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0) abort();
}

EditorUI::EditorUI(RendererInUI& renderer, Window& window)
    : renderer(renderer),
      window(window),
      preview_widget(renderer),
      scene_widget(renderer) {
  // Step 1: Setup Vulkan
  vkb::Device& device = renderer.get_device();
  const vkb::PhysicalDevice& gpu = device.get_gpu();
  vkb::Instance& instance = gpu.get_instance();
  uint32_t queue_family = renderer.get_ui_queue_family();
  VkQueue queue = renderer.get_ui_queue();
  VkDescriptorPool descriptor_pool = renderer.get_descriptor_pool();

  // Step 2: Setup Vulkan Window
  wd.Surface = renderer.request_surface_with_ownershipe();
  wd.SurfaceFormat = renderer.get_surface_format();
  wd.PresentMode = renderer.get_present_mode();
  uint32_t minImageCount = renderer.get_surface_capabilities().minImageCount;

  // Create SwapChain, RenderPass, Framebuffer, etc.
  IM_ASSERT(minImageCount >= 2);
  Window::Extent extent = window.get_extent();
  ImGui_ImplVulkanH_CreateOrResizeWindow(
      instance.get_handle(), gpu.get_handle(), device.get_handle(), &wd,
      queue_family, nullptr, extent.width, extent.height, minImageCount);

  if (!glfwVulkanSupported()) {
    throw std::runtime_error("GLFW: Vulkan Not Supported");
  }

  // Step 3: Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.FontGlobalScale = 2.0f;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(window.get_handle(), true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = instance.get_handle();
  init_info.PhysicalDevice = gpu.get_handle();
  init_info.Device = device.get_handle();
  init_info.QueueFamily = queue_family;
  init_info.Queue = queue;
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = descriptor_pool;
  init_info.Subpass = 0;
  init_info.MinImageCount = minImageCount;
  init_info.ImageCount = wd.ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info, wd.RenderPass);

  // // Step 4: Upload Fonts
  // {
  //   // Use any command queue
  //   VkCommandPool command_pool = wd.Frames[wd.FrameIndex].CommandPool;
  //   VkCommandBuffer command_buffer = wd.Frames[wd.FrameIndex].CommandBuffer;

  //   VkResult err = vkResetCommandPool(device.get_handle(), command_pool, 0);
  //   check_vk_result(err);
  //   VkCommandBufferBeginInfo begin_info = {};
  //   begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  //   begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  //   err = vkBeginCommandBuffer(command_buffer, &begin_info);
  //   check_vk_result(err);

  //   ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

  //   VkSubmitInfo end_info = {};
  //   end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  //   end_info.commandBufferCount = 1;
  //   end_info.pCommandBuffers = &command_buffer;
  //   err = vkEndCommandBuffer(command_buffer);
  //   check_vk_result(err);
  //   err = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);
  //   check_vk_result(err);

  //   err = vkDeviceWaitIdle(device.get_handle());
  //   check_vk_result(err);
  //   ImGui_ImplVulkan_DestroyFontUploadObjects();
  // }
}

EditorUI::~EditorUI() {
  VkDevice device = renderer.get_device().get_handle();
  VkInstance instance =
      renderer.get_device().get_gpu().get_instance().get_handle();

  VkResult err = vkDeviceWaitIdle(device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  ImGui_ImplVulkanH_DestroyWindow(instance, device, &wd, nullptr);
}

void EditorUI::prepare() {
  preview_widget.add_texture(renderer.get_sampler(), renderer.get_image_view());
}

void EditorUI::draw() {
  // Resize swap chain?
  if (swapchain_should_rebuild) {
    const Window::Extent& extent = window.get_extent();
    if (extent.width > 0 && extent.height > 0) {
      auto& device = renderer.get_device();
      auto& gpu = device.get_gpu();
      auto& instance = gpu.get_instance();
      auto capabilities = renderer.get_surface_capabilities();

      ImGui_ImplVulkan_SetMinImageCount(capabilities.minImageCount);
      ImGui_ImplVulkanH_CreateOrResizeWindow(
          instance.get_handle(), gpu.get_handle(), device.get_handle(), &wd,
          renderer.get_ui_queue_family(), nullptr, extent.width, extent.height,
          capabilities.minImageCount);
      wd.FrameIndex = 0;
      swapchain_should_rebuild = false;
    }
  }

  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static float f = 0.0f;
  static int counter = 0;
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiIO& io = ImGui::GetIO();

  ImGui::ShowDemoWindow();

  preview_widget.show();
  scene_widget.show();

  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();
  const bool is_minimized =
      (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  if (!is_minimized) {
    wd.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    wd.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    wd.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    wd.ClearValue.color.float32[3] = clear_color.w;
    frame_render(draw_data);
    frame_present();
  }
}

void EditorUI::frame_render(ImDrawData* draw_data) {
  VkDevice device = renderer.get_device().get_handle();
  VkQueue queue = renderer.get_ui_queue();

  VkResult err;

  VkSemaphore image_acquired_semaphore =
      wd.FrameSemaphores[wd.SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
      wd.FrameSemaphores[wd.SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(device, wd.Swapchain, UINT64_MAX,
                              image_acquired_semaphore, VK_NULL_HANDLE,
                              &wd.FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapchain_should_rebuild = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];
  {
    err = vkWaitForFences(
        device, 1, &fd->Fence, VK_TRUE,
        UINT64_MAX);  // wait indefinitely instead of periodically checking
    check_vk_result(err);

    err = vkResetFences(device, 1, &fd->Fence);
    check_vk_result(err);
  }
  {
    err = vkResetCommandPool(device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }
  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = wd.RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = wd.Width;
    info.renderArea.extent.height = wd.Height;
    info.clearValueCount = 1;
    info.pClearValues = &wd.ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

  // Submit command buffer
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &image_acquired_semaphore;
    info.pWaitDstStageMask = &wait_stage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    check_vk_result(err);
    err = vkQueueSubmit(queue, 1, &info, fd->Fence);
    check_vk_result(err);
  }
}

void EditorUI::frame_present() {
  VkQueue queue = renderer.get_ui_queue();

  if (swapchain_should_rebuild) return;
  VkSemaphore render_complete_semaphore =
      wd.FrameSemaphores[wd.SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd.Swapchain;
  info.pImageIndices = &wd.FrameIndex;
  VkResult err = vkQueuePresentKHR(queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapchain_should_rebuild = true;
    return;
  }
  check_vk_result(err);
  wd.SemaphoreIndex =
      (wd.SemaphoreIndex + 1) %
      wd.ImageCount;  // Now we can use the next set of semaphores
}
