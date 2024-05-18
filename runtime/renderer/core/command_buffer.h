#pragma once

#include "runtime/renderer/core/buffer.h"
#include "runtime/renderer/core/descriptor_set_layout.h"
#include "runtime/renderer/core/render_pass.h"
#include "runtime/renderer/rendering/pipeline_state.h"
#include "runtime/renderer/resource_binding_state.h"
#include "vulkan_resource.h"

namespace vkb {

class CommandPool;
class Framebuffer;
class PipelineLayout;
class PipelineState;
class RenderTarget;
class Subpass;
struct LightingState;

/// @brief Helper class to manage and record a command buffer, building and
/// keeping track of pipeline state and resource bindings
class CommandBuffer
    : public core::VulkanResource<VkCommandBuffer,
                                  VK_OBJECT_TYPE_COMMAND_BUFFER> {
 public:
  enum class ResetMode { ResetPool, ResetIndividually, AlwaysAllocate };

  /// @brief Helper structure used to track render pass state
  struct RenderPassBinding {
    const RenderPass *render_pass;
    const Framebuffer *framebuffer;
  };

  CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level);

  CommandBuffer(const CommandBuffer &) = delete;

  CommandBuffer(CommandBuffer &&other);

  ~CommandBuffer();

  CommandBuffer &operator=(const CommandBuffer &) = delete;

  CommandBuffer &operator=(CommandBuffer &&) = delete;

  /// @brief Flushes the command buffer, pushing the new changes
  /// @param pipeline_bind_point The type of pipeline we want to flush
  void flush(VkPipelineBindPoint pipeline_bind_point);

  /// @brief Sets the command buffer so that it is ready for recording
  ///        If it is a secondary command buffer, a pointer to the primary
  ///        command buffer it inherits from must be provided
  /// @param flags Usage behavior for the command buffer
  /// @param primary_cmd_buf (optional)
  /// @return Whether it succeeded or not
  VkResult begin(VkCommandBufferUsageFlags flags,
                 CommandBuffer *primary_cmd_buf = nullptr);

  /// @brief Sets the command buffer so that it is ready for recording
  ///        If it is a secondary command buffer, pointers to the render pass
  ///        and framebuffer as well as subpass index must be provided
  /// @param flags Usage behavior for the command buffer
  /// @param render_pass
  /// @param framebuffer
  /// @param subpass_index
  /// @return Whether it succeeded or not
  VkResult begin(VkCommandBufferUsageFlags flags, const RenderPass *render_pass,
                 const Framebuffer *framebuffer, uint32_t subpass_index);

  VkResult end();

  void begin_render_pass(
      const RenderTarget &render_target,
      const std::vector<LoadStoreInfo> &load_store_infos,
      const std::vector<VkClearValue> &clear_values,
      const std::vector<std::unique_ptr<Subpass>> &subpasses,
      VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

  void begin_render_pass(
      const RenderTarget &render_target, const RenderPass &render_pass,
      const Framebuffer &framebuffer,
      const std::vector<VkClearValue> &clear_values,
      VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

  void next_subpass();

  void end_render_pass();

  void bind_pipeline_layout(PipelineLayout &pipeline_layout);

  template <class T>
  void set_specialization_constant(uint32_t constant_id, const T &data);

  void set_specialization_constant(uint32_t constant_id,
                                   const std::vector<uint8_t> &data);

  /// @brief Records byte data into the command buffer to be pushed as push
  /// constants to each draw call
  /// @param values The byte data to store
  void push_constants(const std::vector<uint8_t> &values);

  template <typename T>
  void push_constants(const T &value) {
    auto data = to_bytes(value);

    uint32_t size = to_u32(stored_push_constants.size() + data.size());

    if (size > max_push_constants_size) {
      LOGE("Push constant limit exceeded ({} / {} bytes)", size,
           max_push_constants_size);
      throw std::runtime_error("Cannot overflow push constant limit");
    }

    stored_push_constants.insert(stored_push_constants.end(), data.begin(),
                                 data.end());
  }

  void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset,
                   VkDeviceSize range, uint32_t set, uint32_t binding,
                   uint32_t array_element);

  void bind_image(const core::ImageView &image_view,
                  const core::Sampler &sampler, uint32_t set, uint32_t binding,
                  uint32_t array_element);

  void bind_image(const core::ImageView &image_view, uint32_t set,
                  uint32_t binding, uint32_t array_element);

  void bind_input(const core::ImageView &image_view, uint32_t set,
                  uint32_t binding, uint32_t array_element);

  void bind_vertex_buffers(
      uint32_t first_binding,
      const std::vector<std::reference_wrapper<const vkb::core::Buffer>>
          &buffers,
      const std::vector<VkDeviceSize> &offsets);

  void bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset,
                         VkIndexType index_type);

  void bind_lighting(LightingState &lighting_state, uint32_t set,
                     uint32_t binding);

  void set_vertex_input_state(const VertexInputState &state_info);

  void set_rasterization_state(const RasterizationState &state_info);

  void set_multisample_state(const MultisampleState &state_info);

  void set_depth_stencil_state(const DepthStencilState &state_info);

  void set_color_blend_state(const ColorBlendState &state_info);

  void set_viewport(uint32_t first_viewport,
                    const std::vector<VkViewport> &viewports);

  void set_scissor(uint32_t first_scissor,
                   const std::vector<VkRect2D> &scissors);

  void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex, uint32_t first_instance);

  void draw_indexed(uint32_t index_count, uint32_t instance_count,
                    uint32_t first_index, int32_t vertex_offset,
                    uint32_t first_instance);

  void copy_buffer_to_image(const core::Buffer &buffer,
                            const core::Image &image,
                            const std::vector<VkBufferImageCopy> &regions);

  void image_memory_barrier(const core::ImageView &image_view,
                            const ImageMemoryBarrier &memory_barrier) const;

  /// @brief Reset the command buffer to a state where it can be recorded to
  /// @param reset_mode How to reset the buffer, should match the one used by
  /// the pool to allocate it
  VkResult reset(ResetMode reset_mode);

  RenderPass &get_render_pass(
      const vkb::RenderTarget &render_target,
      const std::vector<LoadStoreInfo> &load_store_infos,
      const std::vector<std::unique_ptr<Subpass>> &subpasses);

  const VkCommandBufferLevel level;

 private:
  CommandPool &command_pool;
  RenderPassBinding current_render_pass;
  PipelineState pipeline_state;
  ResourceBindingState resource_binding_state;
  std::vector<uint8_t> stored_push_constants;
  uint32_t max_push_constants_size;
  VkExtent2D last_framebuffer_extent{};
  VkExtent2D last_render_area_extent{};
  // If true, it becomes the responsibility of the caller to update ANY
  // descriptor bindings that contain update after bind, as they wont be
  // implicitly updated
  bool update_after_bind{false};
  std::unordered_map<uint32_t, DescriptorSetLayout *>
      descriptor_set_layout_binding_state;

  /// @brief Check that the render area is an optimal size by comparing to the
  /// render area granularity
  const bool is_render_size_optimal(const VkExtent2D &extent,
                                    const VkRect2D &render_area);

  /// @brief Flush the pipeline state
  void flush_pipeline_state(VkPipelineBindPoint pipeline_bind_point);

  /// @brief Flush the descriptor set state
  void flush_descriptor_state(VkPipelineBindPoint pipeline_bind_point);

  /// @brief Flush the push constant state
  void flush_push_constants();
};
}  // namespace vkb