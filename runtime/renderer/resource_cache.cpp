#include "resource_cache.h"

#include <mutex>

#include "runtime/renderer/common/resource_caching.h"
namespace vkb {

namespace {
template <class T, class... A>
T &request_resource(Device &device, ResourceRecord &recorder,
                    std::mutex &resource_mutex,
                    std::unordered_map<std::size_t, T> &resources, A &...args) {
  std::lock_guard<std::mutex> guard(resource_mutex);

  auto &res = request_resource(device, &recorder, resources, args...);

  return res;
}
}  // namespace

ResourceCache::ResourceCache(Device &device) : device(device) {}

// /***************************** XR.Y *****************************/
// /* 为了改写着色器的加载 */
// ShaderModule &ResourceCache::request_shader_module(VkShaderStageFlagBits
// stage,
//                                                    const std::string &path) {
//   std::string entry_point{"main"};
//   return request_resource(device, recorder, shader_module_mutex,
//                           state.shader_modules, stage, path, entry_point);
// }
// /***************************** XR.Y *****************************/

#define OFFLINE
ShaderModule &ResourceCache::request_shader_module(
    VkShaderStageFlagBits stage, const ShaderSource &glsl_source,
    const ShaderVariant &shader_variant) {
  std::string entry_point{"main"};
#ifndef OFFLINE
  return request_resource(device, recorder, shader_module_mutex,
                          state.shader_modules, stage, glsl_source, entry_point,
                          shader_variant);
#else
  return request_resource(
      device, recorder, shader_module_mutex, state.shader_modules, stage,
      glsl_source.get_filename(), entry_point, shader_variant);
#endif
}
#undef OFFLINE

PipelineLayout &ResourceCache::request_pipeline_layout(
    const std::vector<ShaderModule *> &shader_modules) {
  return request_resource(device, recorder, pipeline_layout_mutex,
                          state.pipeline_layouts, shader_modules);
}

DescriptorSetLayout &ResourceCache::request_descriptor_set_layout(
    const uint32_t set_index, const std::vector<ShaderModule *> &shader_modules,
    const std::vector<ShaderResource> &set_resources) {
  return request_resource(device, recorder, descriptor_set_layout_mutex,
                          state.descriptor_set_layouts, set_index,
                          shader_modules, set_resources);
}

GraphicsPipeline &ResourceCache::request_graphics_pipeline(
    PipelineState &pipeline_state) {
  return request_resource(device, recorder, graphics_pipeline_mutex,
                          state.graphics_pipelines, pipeline_cache,
                          pipeline_state);
}

ComputePipeline &ResourceCache::request_compute_pipeline(
    PipelineState &pipeline_state) {
  return request_resource(device, recorder, compute_pipeline_mutex,
                          state.compute_pipelines, pipeline_cache,
                          pipeline_state);
}

RenderPass &ResourceCache::request_render_pass(
    const std::vector<Attachment> &attachments,
    const std::vector<LoadStoreInfo> &load_store_infos,
    const std::vector<SubpassInfo> &subpasses) {
  return request_resource(device, recorder, render_pass_mutex,
                          state.render_passes, attachments, load_store_infos,
                          subpasses);
}

Framebuffer &ResourceCache::request_framebuffer(
    const RenderTarget &render_target, const RenderPass &render_pass) {
  return request_resource(device, recorder, framebuffer_mutex,
                          state.framebuffers, render_target, render_pass);
}

void ResourceCache::clear_pipelines() {
  state.graphics_pipelines.clear();
  state.compute_pipelines.clear();
}

void ResourceCache::clear_framebuffers() { state.framebuffers.clear(); }

void ResourceCache::clear() {
  state.shader_modules.clear();
  state.pipeline_layouts.clear();
  state.descriptor_sets.clear();
  state.descriptor_set_layouts.clear();
  state.render_passes.clear();
  clear_pipelines();
  clear_framebuffers();
}
}  // namespace vkb