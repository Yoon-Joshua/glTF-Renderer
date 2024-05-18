#include "component.h"

#include <algorithm>

#include "node.h"

namespace sg {
Component::Component(const std::string &name) : name{name} {}

const std::string &Component::get_name() const { return name; }
}  // namespace sg
