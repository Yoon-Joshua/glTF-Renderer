#include "aabb.h"

namespace sg {
AABB::AABB() { reset(); }

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max) : min{min}, max{max} {}

std::type_index AABB::get_type() { return typeid(AABB); }

void AABB::update(const glm::vec3 &point) {
  min = glm::min(min, point);
  max = glm::max(max, point);
}

void AABB::update(const std::vector<glm::vec3> &vertex_data,
                  const std::vector<uint16_t> &index_data) {
  // Check if submesh is indexed
  if (index_data.size() > 0) {
    // Update bounding box for each indexed vertex
    for (size_t index_id = 0; index_id < index_data.size(); index_id++) {
      update(vertex_data[index_data[index_id]]);
    }
  } else {
    // Update bounding box for each vertex
    for (size_t vertex_id = 0; vertex_id < vertex_data.size(); vertex_id++) {
      update(vertex_data[vertex_id]);
    }
  }
}

void AABB::transform(glm::mat4 &transform) {
  min = max = glm::vec4(min, 1.0f) * transform;

  // Update bounding box for the remaining 7 corners of the box
  update(glm::vec4(min.x, min.y, max.z, 1.0f) * transform);
  update(glm::vec4(min.x, max.y, min.z, 1.0f) * transform);
  update(glm::vec4(min.x, max.y, max.z, 1.0f) * transform);
  update(glm::vec4(max.x, min.y, min.z, 1.0f) * transform);
  update(glm::vec4(max.x, min.y, max.z, 1.0f) * transform);
  update(glm::vec4(max.x, max.y, min.z, 1.0f) * transform);
  update(glm::vec4(max, 1.0f) * transform);
}

glm::vec3 AABB::get_scale() const { return (max - min); }

glm::vec3 AABB::get_center() const { return (min + max) * 0.5f; }

glm::vec3 AABB::get_min() const { return min; }

glm::vec3 AABB::get_max() const { return max; }

void AABB::reset() {
  min = std::numeric_limits<glm::vec3>::max();
  max = std::numeric_limits<glm::vec3>::min();
}
};  // namespace sg