#pragma once
#include <Volk/volk.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace vkb {

class DescriptorPool;
class Device;
class ShaderModule;
struct ShaderResource;

/// @brief Caches DescriptorSet objects for the shader's set index.
/// Creates a DescriptorPool to allocate the DescriptorSet objects
class DescriptorSetLayout {
 public:
  /// @brief Creates a descriptor set layout from a set of resources
  /// @param device A valid Vulkan device
  /// @param set_index The descriptor set index this layout maps to
  /// @param shader_modules The shader modules this set layout will be used for
  /// @param resource_set A grouping of shader resources belonging to the same
  /// set
  DescriptorSetLayout(Device &device, const uint32_t set_index,
                      const std::vector<ShaderModule *> &shader_modules,
                      const std::vector<ShaderResource> &resource_set);

  DescriptorSetLayout(const DescriptorSetLayout &) = delete;

  DescriptorSetLayout(DescriptorSetLayout &&other);

  ~DescriptorSetLayout();

  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

  VkDescriptorSetLayout get_handle() const;

  const uint32_t get_index() const;

  const std::vector<VkDescriptorSetLayoutBinding> &get_bindings() const;

  std::unique_ptr<VkDescriptorSetLayoutBinding> get_layout_binding(
      const uint32_t binding_index) const;

  std::unique_ptr<VkDescriptorSetLayoutBinding> get_layout_binding(
      const std::string &name) const;

  const std::vector<VkDescriptorBindingFlagsEXT> &get_binding_flags() const;

  VkDescriptorBindingFlagsEXT get_layout_binding_flag(
      const uint32_t binding_index) const;

  const std::vector<ShaderModule *> &get_shader_modules() const;

 private:
  Device &device;

  VkDescriptorSetLayout handle{VK_NULL_HANDLE};

  const uint32_t set_index;

  std::vector<VkDescriptorSetLayoutBinding> bindings;

  std::vector<VkDescriptorBindingFlagsEXT> binding_flags;

  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_lookup;

  std::unordered_map<uint32_t, VkDescriptorBindingFlagsEXT>
      binding_flags_lookup;

  std::unordered_map<std::string, uint32_t> resources_lookup;

  std::vector<ShaderModule *> shader_modules;
};
}  // namespace vkb