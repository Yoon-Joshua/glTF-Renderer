#include <glm/glm.hpp>
#include <string>
#include <vector>

class Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};


class SubMesh {
 private:
  std::vector<size_t> indices;
  std::vector<Vertex> vertices;
};