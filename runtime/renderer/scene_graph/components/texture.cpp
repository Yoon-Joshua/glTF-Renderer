#include "texture.h"

namespace sg {
Texture::Texture(const std::string &name) : Component{name} {}

std::type_index Texture::get_type() { return typeid(Texture); }

void Texture::set_image(Image &i) { image = &i; }

Image *Texture::get_image() { return image; }

void Texture::set_sampler(Sampler &s) { sampler = &s; }

Sampler *Texture::get_sampler() {
  assert(sampler && "Texture has no sampler");
  return sampler;
}
}  // namespace sg