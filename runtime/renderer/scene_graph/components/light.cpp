#include "runtime/renderer/scene_graph/components/light.h"

namespace sg {
Light::Light(const std::string &name) : Component{name} {}

std::type_index Light::get_type() { return typeid(Light); }

void Light::set_node(Node &n) { node = &n; }

Node *Light::get_node() { return node; }

void Light::set_light_type(const LightType &type) { this->light_type = type; }

const LightType &Light::get_light_type() { return light_type; }

void Light::set_properties(const LightProperties &properties) {
  this->properties = properties;
}

const LightProperties &Light::get_properties() { return properties; }
}  // namespace sg