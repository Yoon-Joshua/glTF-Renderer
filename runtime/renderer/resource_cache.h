#pragma once

#include <Volk/volk.h>

#include <mutex>
#include <unordered_map>

#include "runtime/renderer/core/descriptor_set.h"
#include "runtime/renderer/core/descriptor_set_layout.h"
#include "runtime/renderer/core/framebuffer.h"
#include "runtime/renderer/core/pipeline.h"
#include "runtime/renderer/core/shader_module.h"
#include "runtime/renderer/resource_record.h"

namespace vkb {

class Device;

/// @brief Struct to hold the internal state of the Resource Cache
struct ResourceCacheState {
  std::unordered_map<std::size_t, ShaderModule> shader_modules;
  std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;
  std::unordered_map<std::size_t, DescriptorSetLayout> descriptor_set_layouts;
  // std::unordered_map<std::size_t, DescriptorPool> descriptor_pools;
  std::unordered_map<std::size_t, RenderPass> render_passes;
  std::unordered_map<std::size_t, GraphicsPipeline> graphics_pipelines;
  std::unordered_map<std::size_t, ComputePipeline> compute_pipelines;
  std::unordered_map<std::size_t, DescriptorSet> descriptor_sets;
  std::unordered_map<std::size_t, Framebuffer> framebuffers;
};

/**
 * @brief Cache all sorts of Vulkan objects specific to a Vulkan device.
 * Supports serialization and deserialization of cached resources.
 * There is only one cache for all these objects, with several unordered_map of
 * hash indices and objects. For every object requested, there is a templated
 * version on request_resource. Some objects may need building if they are not
 * found in the cache.
 *
 * The resource cache is also linked with ResourceRecord and ResourceReplay.
 * Replay can warm-up the cache on app startup by creating all necessary
 * objects. The cache holds pointers to objects and has a mapping from such
 * pointers to hashes. It can only be destroyed in bulk, single elements cannot
 * be removed.
 */

class ResourceCache {
 public:
  ResourceCache(Device &device);

  ResourceCache(const ResourceCache &) = delete;

  ResourceCache(ResourceCache &&) = delete;

  ResourceCache &operator=(const ResourceCache &) = delete;

  ResourceCache &operator=(ResourceCache &&) = delete;

  ShaderModule &request_shader_module(VkShaderStageFlagBits stage,
                                      const ShaderSource &glsl_source,
                                      const ShaderVariant &shader_variant);

  PipelineLayout &request_pipeline_layout(
      const std::vector<ShaderModule *> &shader_modules);

  DescriptorSetLayout &request_descriptor_set_layout(
      const uint32_t set_index,
      const std::vector<ShaderModule *> &shader_modules,
      const std::vector<ShaderResource> &set_resources);

  GraphicsPipeline &request_graphics_pipeline(PipelineState &pipeline_state);

  ComputePipeline &request_compute_pipeline(PipelineState &pipeline_state);

  RenderPass &request_render_pass(
      const std::vector<Attachment> &attachments,
      const std::vector<LoadStoreInfo> &load_store_infos,
      const std::vector<SubpassInfo> &subpasses);

  Framebuffer &request_framebuffer(const RenderTarget &render_target,
                                   const RenderPass &render_pass);

  void clear_pipelines();

  void clear_framebuffers();

  void clear();

 private:
  Device &device;

  ResourceRecord recorder;

  VkPipelineCache pipeline_cache{VK_NULL_HANDLE};
  ResourceCacheState state;

  std::mutex pipeline_layout_mutex;
  std::mutex shader_module_mutex;
  std::mutex descriptor_set_layout_mutex;
  std::mutex graphics_pipeline_mutex;
  std::mutex render_pass_mutex;
  std::mutex compute_pipeline_mutex;
  std::mutex framebuffer_mutex;
};
}  // namespace vkb