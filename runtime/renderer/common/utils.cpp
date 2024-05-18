#include "utils.h"

#include "runtime/renderer/scene_graph/components/camera.h"
#include "runtime/renderer/scene_graph/node.h"
#include "runtime/renderer/scene_graph/scripts/free_camera.h"

namespace vkb {
std::string get_extension(const std::string &uri) {
  auto dot_pos = uri.find_last_of('.');
  if (dot_pos == std::string::npos) {
    throw std::runtime_error{"Uri has no extension"};
  }

  return uri.substr(dot_pos + 1);
}

std::string to_snake_case(const std::string &text) {
  std::stringstream result;

  for (const auto ch : text) {
    if (std::isalpha(ch)) {
      if (std::isspace(ch)) {
        result << "_";
      } else {
        if (std::isupper(ch)) {
          result << "_";
        }

        result << static_cast<char>(std::tolower(ch));
      }
    } else {
      result << ch;
    }
  }

  return result.str();
}

}  // namespace vkb

sg::Light &add_light(sg::Scene &scene, sg::LightType type,
                     const glm::vec3 &position, const glm::quat &rotation,
                     const sg::LightProperties &props, sg::Node *parent_node) {
  auto light_ptr = std::make_unique<sg::Light>("light");
  auto node = std::make_unique<sg::Node>(-1, "light node");

  if (parent_node) {
    node->set_parent(*parent_node);
  }

  light_ptr->set_node(*node);
  light_ptr->set_light_type(type);
  light_ptr->set_properties(props);

  auto &t = node->get_transform();
  t.set_translation(position);
  t.set_rotation(rotation);

  // Storing the light component because the unique_ptr will be moved to the
  // scene
  auto &light = *light_ptr;

  node->set_component(light);
  scene.add_child(*node);
  scene.add_component(std::move(light_ptr));
  scene.add_node(std::move(node));

  return light;
}

sg::Light &add_directional_light(sg::Scene &scene, const glm::quat &rotation,
                                 const sg::LightProperties &props,
                                 sg::Node *parent_node) {
  return add_light(scene, sg::LightType::Directional, {}, rotation, props,
                   parent_node);
}

sg::Node &add_free_camera(sg::Scene &scene, const std::string &node_name,
                          VkExtent2D extent) {
  auto camera_node = scene.find_node(node_name);

  if (!camera_node) {
    LOGW("Camera node `{}` not found. Looking for `default_camera` node.",
         node_name.c_str());

    camera_node = scene.find_node("default_camera");
  }

  if (!camera_node) {
    throw std::runtime_error("Camera node with name `" + node_name +
                             "` not found.");
  }

  if (!camera_node->has_component<sg::Camera>()) {
    throw std::runtime_error("No camera component found for `" + node_name +
                             "` node.");
  }

  auto free_camera_script = std::make_unique<sg::FreeCamera>(*camera_node);

  free_camera_script->resize(extent.width, extent.height);

  scene.add_component(std::move(free_camera_script), *camera_node);

  return *camera_node;
}