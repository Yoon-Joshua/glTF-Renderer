#pragma once
#include <Volk/volk.h>

#include <cstdint>
#include <memory>

#include "runtime/renderer/core/buffer.h"
#include "runtime/renderer/core/shader_module.h"
#include "runtime/renderer/scene_graph/component.h"

namespace sg {

class Material;

struct VertexAttribute {
  VkFormat format = VK_FORMAT_UNDEFINED;
  std::uint32_t stride = 0;
  std::uint32_t offset = 0;
};

class SubMesh : public Component {
 public:
  SubMesh(const std::string &name = {});

  virtual ~SubMesh() = default;

  virtual std::type_index get_type() override;

  VkIndexType index_type{};

  std::uint32_t index_offset = 0;

  std::uint32_t vertices_count = 0;

  std::uint32_t vertex_indices = 0;

  std::unordered_map<std::string, vkb::core::Buffer> vertex_buffers;

  std::unique_ptr<vkb::core::Buffer> index_buffer;

  void set_attribute(const std::string &name, const VertexAttribute &attribute);

  bool get_attribute(const std::string &name, VertexAttribute &attribute) const;

  void set_material(const Material &material);

  const Material *get_material() const;

  const vkb::ShaderVariant &get_shader_variant() const;

  vkb::ShaderVariant &get_mut_shader_variant();

 private:
  std::unordered_map<std::string, VertexAttribute> vertex_attributes;

  const Material *material{nullptr};

  vkb::ShaderVariant shader_variant;

  void compute_shader_variant();
};
}  // namespace sg