#pragma once

#include "runtime/renderer/common/glm_common.h"
#include "runtime/renderer/rendering/subpass.h"

// This value is per type of light that we feed into the shader
#define MAX_DEFERRED_LIGHT_COUNT 32

namespace sg {
class Camera;
class Light;
class Scene;
}  // namespace sg

namespace vkb {
/// @brief Light uniform structure for lighting shader
/// Inverse view projection matrix and inverse resolution vector are used in
/// lighting pass to reconstruct position from depth and frag coord
struct alignas(16) LightUniform {
  alignas(16) glm::mat4 inv_view_proj;
  alignas(16) glm::vec2 inv_resolution;
  alignas(16) glm::vec4 camera_pos;
};

struct alignas(16) DeferredLights {
  Light directional_lights[MAX_DEFERRED_LIGHT_COUNT];
  Light point_lights[MAX_DEFERRED_LIGHT_COUNT];
  Light spot_lights[MAX_DEFERRED_LIGHT_COUNT];
};

#define XRY
#ifdef XRY
enum class LightType { SPOT, POINT, DIRECTIONAL };
#endif

/// @brief Lighting pass of Deferred Rendering
class LightingSubpass : public Subpass {
 public:
  LightingSubpass(RenderContext &render_context, ShaderSource &&vertex_shader,
                  ShaderSource &&fragment_shader, sg::Camera &camera,
                  sg::Scene &scene);

  virtual void prepare() override;

  void draw(CommandBuffer &command_buffer) override;

 private:
  sg::Camera &camera;

  sg::Scene &scene;

  ShaderVariant lighting_variant;
};

}  // namespace vkb
