#pragma once
#include <glm/glm.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "runtime/renderer/scene_graph/component.h"
namespace sg {

class Texture;

/// @brief How the alpha value of the main factor and texture should be
/// interpreted
enum class AlphaMode {
  /// Alpha value is ignored
  Opaque,
  /// Either full opaque or fully transparent
  Mask,
  /// Output is combined with the background
  Blend
};

class Material : public Component {
 public:
  Material(const std::string &name);

  Material(Material &&other) = default;

  virtual ~Material() = default;

  virtual std::type_index get_type() override;

  std::unordered_map<std::string, Texture *> textures;

  /// Emissive color of the material
  glm::vec3 emissive{0.0f, 0.0f, 0.0f};

  /// Whether the material is double sided
  bool double_sided{false};

  /// Cutoff threshold when in Mask mode
  float alpha_cutoff{0.5f};

  /// Alpha rendering mode
  AlphaMode alpha_mode{AlphaMode::Opaque};
};

}  // namespace sg