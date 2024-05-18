#include "material.h"
namespace sg {
Material::Material(const std::string &name) : Component{name} {}

std::type_index Material::get_type() { return typeid(Material); }

}  // namespace sg
