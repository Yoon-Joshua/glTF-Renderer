#pragma once

#include <glm/glm.hpp>

namespace vkb {
enum CameraType { LookAt, FirstPerson };

class Camera {
 public:
  void update(float deltaTime);

  CameraType type = CameraType::LookAt;

  glm::vec3 rotation = glm::vec3();
  glm::vec3 position = glm::vec3();

  float rotation_speed = 1.0f;
  float translation_speed = 1.0f;

  bool updated = false;

  struct {
    glm::mat4 perspective;
    glm::mat4 view;
  } matrices;

  struct {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
  } keys;

  bool moving();

  void set_perspective(float fov, float aspect, float znear, float zfar);

  void update_aspect_ratio(float aspect);

  void set_rotation(const glm::vec3 &rotation);

  void rotate(const glm::vec3 &delta);

  void set_translation(const glm::vec3 &translation);

 private:
  float fov;
  float znear, zfar;
  void update_view_matrix();
};

}  // namespace vkb