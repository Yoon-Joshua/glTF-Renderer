#include "runtime/renderer/camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace vkb {

void Camera::update(float deltaTime) {
  updated = false;
  if (type == CameraType::FirstPerson) {
    if (moving()) {
      glm::vec3 front;
      front.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
      front.y = sin(glm::radians(rotation.x));
      front.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
      front = glm::normalize(front);

      float move_speed = deltaTime * translation_speed;

      if (keys.up) {
        position += front * move_speed;
      }
      if (keys.down) {
        position -= front * move_speed;
      }
      if (keys.left) {
        position -=
            glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) *
            move_speed;
      }
      if (keys.right) {
        position +=
            glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) *
            move_speed;
      }

      update_view_matrix();
    }
  }
}

bool Camera::moving() {
  return keys.left || keys.right || keys.up || keys.down;
}

void Camera::set_perspective(float fov, float aspect, float znear, float zfar) {
  this->fov = fov;
  this->znear = znear;
  this->zfar = zfar;
  matrices.perspective =
      glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void Camera::update_aspect_ratio(float aspect) {
  matrices.perspective =
      glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void Camera::set_rotation(const glm::vec3 &rotation) {
    this->rotation = rotation;
    update_view_matrix();
}

void Camera::rotate(const glm::vec3 &delta) {
    this->rotation += delta;
    update_view_matrix();
}

void Camera::set_translation(const glm::vec3 &translation) {
    this->position = translation;
    update_view_matrix();
}

void Camera::update_view_matrix() {
  glm::mat4 rotation_matrix = glm::mat4(1.0f);
  glm::mat4 transformation_matrix;
  rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.x),
                                glm::vec3(1.0f, 0.0f, 0.0f));
  rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.y),
                                glm::vec3(0.0f, 1.0f, 0.0f));
  rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.z),
                                glm::vec3(0.0f, 0.0f, 1.0f));

  transformation_matrix = glm::translate(glm::mat4(1.0f), position);

  if (type == CameraType::FirstPerson) {
    matrices.view = rotation_matrix * transformation_matrix;
  } else {
    matrices.view = transformation_matrix * rotation_matrix;
  }

  updated = true;
}

}  // namespace vkb