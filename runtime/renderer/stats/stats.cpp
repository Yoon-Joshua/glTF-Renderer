#include "stats.h"

#include <cassert>

namespace vkb {
Stats::Stats(RenderContext &rc, size_t bs)
    : render_context(rc), buffer_size(bs) {
  assert(buffer_size >= 2 && "Buffers size should be greater than 2");
}

Stats::~Stats() {}

void Stats::begin_sampling(CommandBuffer &cb) {}

void Stats::end_sampling(CommandBuffer &cb) {}
}  // namespace vkb