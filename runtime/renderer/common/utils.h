#pragma once
#include "runtime/renderer/scene_graph/scene.h"

namespace vkb {
/// @brief Extracts the extension from an uri
/// @param uri An uniform Resource Identifier
/// @return The extension
std::string get_extension(const std::string &uri);

/// @param name String to convert to snake case
/// @return a snake case version of the string
std::string to_snake_case(const std::string &name);

}  // namespace vkb

/// @brief Adds a light to the scene with the specified parameters
/// @param scene The scene to add the light to
/// @param type The light type
/// @param position The position of the light
/// @param rotation The rotation of the light
/// @param props The light properties, such as color and intensity
/// @param parent_node The parent node for the line, defaults to root
/// @return The newly created light component
sg::Light &add_light(sg::Scene &scene, sg::LightType type,
                     const glm::vec3 &position, const glm::quat &rotation = {},
                     const sg::LightProperties &props = {},
                     sg::Node *parent_node = nullptr);

/// @brief Adds a directional light to the scene with the specified parameters
/// @param scene The scene to add the light to
/// @param rotation The rotation of the light
/// @param props The light properties, such as color and intensity
/// @param parent_node The parent node for the line, defaults to root
/// @return The newly created light component
sg::Light &add_directional_light(sg::Scene &scene, const glm::quat &rotation,
                                 const sg::LightProperties &props = {},
                                 sg::Node *parent_node = nullptr);

/**
 * @brief Add free camera script to a node with a camera object.
 *        Fallback to the default_camera if node not found.
 * @param scene The scene to add the camera to
 * @param node_name The scene node name
 * @param extent The initial resolution of the camera
 * @return Node where the script was attached as component
 */
sg::Node &add_free_camera(sg::Scene &scene, const std::string &node_name,
                          VkExtent2D extent);