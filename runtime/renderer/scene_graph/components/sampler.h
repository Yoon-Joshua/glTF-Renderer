#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "runtime/renderer/core/sampler.h"
#include "runtime/renderer/scene_graph/component.h"

namespace sg {
class Sampler : public Component {
 public:
  Sampler(const std::string &name, vkb::core::Sampler &&vk_sampler);

  Sampler(Sampler &&other) = default;

  virtual ~Sampler() = default;

  virtual std::type_index get_type() override;

  vkb::core::Sampler vk_sampler;
};
}  // namespace sg
