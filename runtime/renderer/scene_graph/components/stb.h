#pragma once

#include "runtime/renderer/scene_graph/components/image.h"

namespace sg {
class Stb : public Image {
 public:
  Stb(const std::string &name, const std::vector<uint8_t> &data,
      ContentType content_type);

  virtual ~Stb() = default;
};

}  // namespace sg