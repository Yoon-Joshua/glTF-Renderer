#pragma once

#include "runtime/renderer/core/pipeline_layout.h"
#include "runtime/renderer/rendering/render_context.h"
#include "runtime/renderer/rendering/subpass.h"

namespace sg {
class Scene;
class Node;
class Mesh;
class SubMesh;
class Camera;
}  // namespace sg

namespace vkb {

/// @brief Global uniform structure for base shader
struct alignas(16) GlobalUniform {
  glm::mat4 model;
  glm::mat4 camera_view_proj;
  glm::vec3 camera_position;
};

/// @brief PBR material uniform for base shader
struct PBRMaterialUniform {
  glm::vec4 base_color_factor;
  float metallic_factor;
  float roughness_factor;
};

class GeometrySubpass : public Subpass {
 public:
  /// @brief Constructs a subpass for the geometry pass of Deferred rendering
  /// @param render_context Render context
  /// @param vertex_shader Vertex shader source
  /// @param fragment_shader Fragment shader source
  /// @param scene Scene to render on this subpass
  /// @param camera Camera used to look at the scene
  GeometrySubpass(RenderContext &render_context, ShaderSource &&vertex_shader,
                  ShaderSource &&fragment_shader, sg::Scene &scene,
                  sg::Camera &camera);

  virtual ~GeometrySubpass() = default;

  virtual void prepare() override;

  /// @brief Record draw commands
  virtual void draw(CommandBuffer &command_buffer) override;

  /// @brief Thread index to use for allocating resources
  void set_thread_index(uint32_t index);

 protected:
  virtual void update_uniform(CommandBuffer &command_buffer, sg::Node &node,
                              size_t thread_index);

  void draw_submesh(CommandBuffer &command_buffer, sg::SubMesh &sub_mesh,
                    VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE);

  virtual void prepare_pipeline_state(CommandBuffer &command_buffer,
                                      VkFrontFace front_face,
                                      bool double_sided_material);

  virtual PipelineLayout &prepare_pipeline_layout(
      CommandBuffer &command_buffer,
      const std::vector<ShaderModule *> &shader_modules);

  virtual void prepare_push_constants(CommandBuffer &command_buffer,
                                      sg::SubMesh &sub_mesh);

  virtual void draw_submesh_command(CommandBuffer &command_buffer,
                                    sg::SubMesh &sub_mesh);

  /// @brief Sorts objects based on distance from camera and classifies them
  /// into opaque and transparent in the arrays provided
  void get_sorted_nodes(
      std::multimap<float, std::pair<sg::Node *, sg::SubMesh *>> &opaque_nodes,
      std::multimap<float, std::pair<sg::Node *, sg::SubMesh *>>
          &transparent_nodes);

  sg::Camera &camera;

  std::vector<sg::Mesh *> meshes;

  sg::Scene &scene;

  uint32_t thread_index{0};

  RasterizationState base_rasterization_state{};
};
}  // namespace vkb