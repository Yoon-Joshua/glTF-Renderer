#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "runtime/renderer/common/error.h"
#include "runtime/renderer/scene_graph/component.h"
#include "runtime/renderer/scene_graph/components/sub_mesh.h"

namespace sg {
/**
 * @brief Axis Aligned Bounding Box
 */
class AABB : public Component {
 public:
  AABB();

  AABB(const glm::vec3 &min, const glm::vec3 &max);

  virtual ~AABB() = default;

  virtual std::type_index get_type() override;

  /**
   * @brief Update the bounding box based on the given vertex position
   * @param point The 3D position of a point
   */
  void update(const glm::vec3 &point);

  /**
   * @brief Update the bounding box based on the given submesh vertices
   * @param vertex_data The position vertex data
   * @param index_data The index vertex data
   */
  void update(const std::vector<glm::vec3> &vertex_data,
              const std::vector<uint16_t> &index_data);

  /**
   * @brief Apply a given matrix transformation to the bounding box
   * @param transform The matrix transform to apply
   */
  void transform(glm::mat4 &transform);

  /**
   * @brief Scale vector of the bounding box
   * @return vector in 3D space
   */
  glm::vec3 get_scale() const;

  /**
   * @brief Center position of the bounding box
   * @return vector in 3D space
   */
  glm::vec3 get_center() const;

  /**
   * @brief Minimum position of the bounding box
   * @return vector in 3D space
   */
  glm::vec3 get_min() const;

  /**
   * @brief Maximum position of the bounding box
   * @return vector in 3D space
   */
  glm::vec3 get_max() const;

  /**
   * @brief Resets the min and max position coordinates
   */
  void reset();

 private:
  glm::vec3 min;

  glm::vec3 max;
};
}  // namespace sg
