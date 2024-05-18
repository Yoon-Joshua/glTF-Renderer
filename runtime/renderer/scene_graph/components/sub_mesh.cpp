#include "runtime/renderer/scene_graph/components/sub_mesh.h"

#include <typeindex>

#include "runtime/renderer/rendering/subpass.h"
#include "runtime/renderer/scene_graph/components/material.h"
#include "sub_mesh.h"

#include<algorithm>

namespace sg {
SubMesh::SubMesh(const std::string &name) : Component{name} {}

std::type_index SubMesh::get_type() { return typeid(SubMesh); }

void SubMesh::set_attribute(const std::string &attribute_name,
                            const VertexAttribute &attribute) {
  vertex_attributes[attribute_name] = attribute;

  compute_shader_variant();
}

bool SubMesh::get_attribute(const std::string &attribute_name,
                            VertexAttribute &attribute) const {
  auto attrib_it = vertex_attributes.find(attribute_name);

  if (attrib_it == vertex_attributes.end()) {
    return false;
  }

  attribute = attrib_it->second;

  return true;
}

void SubMesh::set_material(const Material &new_material) {
  material = &new_material;

  compute_shader_variant();
}

const Material *SubMesh::get_material() const { return material; }

const vkb::ShaderVariant &SubMesh::get_shader_variant() const {
  return shader_variant;
}

void SubMesh::compute_shader_variant() {
  shader_variant.clear();

  if (material != nullptr) {
    for (auto &texture : material->textures) {
      std::string tex_name = texture.first;
      std::transform(tex_name.begin(), tex_name.end(), tex_name.begin(),
                     ::toupper);

      shader_variant.add_define("HAS_" + tex_name);
    }
  }

  for (auto &attribute : vertex_attributes) {
    std::string attrib_name = attribute.first;
    std::transform(attrib_name.begin(), attrib_name.end(), attrib_name.begin(),
                   ::toupper);
    shader_variant.add_define("HAS_" + attrib_name);
  }
}

vkb::ShaderVariant &SubMesh::get_mut_shader_variant() { return shader_variant; }
}  // namespace sg
