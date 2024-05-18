#pragma once

#include "runtime/renderer/common/helpers.h"
#include "runtime/renderer/common/vk_common.h"
#include "runtime/renderer/core/descriptor_set_layout.h"
#include "runtime/renderer/core/shader_module.h"

namespace vkb {
class Device;
class ShaderModule;
class DescriptorSetLayout;

class PipelineLayout {
 public:
  PipelineLayout(Device &device,
                 const std::vector<ShaderModule *> &shader_modules);

  PipelineLayout(const PipelineLayout &) = delete;

  PipelineLayout(PipelineLayout &&other);

  ~PipelineLayout();

  PipelineLayout &operator=(const PipelineLayout &) = delete;

  PipelineLayout &operator=(PipelineLayout &&) = delete;

  VkPipelineLayout get_handle() const;

  const std::vector<ShaderModule *> &get_shader_modules() const;

  const std::vector<ShaderResource> get_resources(
      const ShaderResourceType &type = ShaderResourceType::All,
      VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;

  const std::unordered_map<uint32_t, std::vector<ShaderResource>> &
  get_shader_sets() const;

  bool has_descriptor_set_layout(const uint32_t set_index) const;

  DescriptorSetLayout &get_descriptor_set_layout(
      const uint32_t set_index) const;

  VkShaderStageFlags get_push_constant_range_stage(uint32_t size,
                                                   uint32_t offset = 0) const;

 private:
  Device &device;

  VkPipelineLayout handle{VK_NULL_HANDLE};

  // The shader modules that this pipeline layout uses
  std::vector<ShaderModule *> shader_modules;

  // The shader resources that this pipeline layout uses, indexed by their name
  std::unordered_map<std::string, ShaderResource> shader_resources;

  // A map of each set and the resources it owns used by the pipeline layout
  std::unordered_map<uint32_t, std::vector<ShaderResource>> shader_sets;

  // The different descriptor set layouts for this pipeline layout
  std::vector<DescriptorSetLayout *> descriptor_set_layouts;
};
}  // namespace vkb
