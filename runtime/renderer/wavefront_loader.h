#pragma once

#include <memory>

namespace sg {
class Camera;
class Image;
class Light;
class Mesh;
class Node;
class PBRMaterial;
class Sampler;
class Scene;
class SubMesh;
class Texture;
}  // namespace sg

namespace vkb {

class Device;

class WaveFrontLoader {
 public:
  WaveFrontLoader(Device const &device);

  std::unique_ptr<sg::Scene> read_scene_from_file(const std::string &file_name);

 private:
  Device const &device;
};

};  // namespace vkb