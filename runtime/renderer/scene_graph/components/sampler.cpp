#include "runtime/renderer/core/sampler.h"

#include "runtime/renderer/scene_graph/components/sampler.h"

namespace sg {
Sampler::Sampler(const std::string &name, vkb::core::Sampler &&vk_sampler)
    : Component{name}, vk_sampler{std::move(vk_sampler)} {}

std::type_index Sampler::get_type() { return typeid(Sampler); }
}  // namespace sg
