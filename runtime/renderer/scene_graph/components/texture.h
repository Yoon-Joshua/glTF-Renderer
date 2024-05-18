#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "runtime/renderer/scene_graph/component.h"
#include "runtime/renderer/scene_graph/components/sampler.h"

namespace sg {
class Image;
class Sampler;

class Texture : public Component {
 public:
  Texture(const std::string &name);

  Texture(Texture &&other) = default;

  virtual ~Texture() = default;

  virtual std::type_index get_type() override;

  void set_image(Image &image);

  Image *get_image();

  void set_sampler(Sampler &sampler);

  Sampler *get_sampler();

 private:
  Image *image{nullptr};

  Sampler *sampler{nullptr};
};
}  // namespace sg
