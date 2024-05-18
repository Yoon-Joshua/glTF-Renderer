#include "lighting_subpass.h"

#include "config/definitions.h"
#include "runtime/renderer/scene_graph/components/camera.h"
#include "runtime/renderer/scene_graph/components/light.h"
#include "runtime/renderer/scene_graph/scene.h"
#define XRY
namespace vkb {
LightingSubpass::LightingSubpass(RenderContext &render_context,
                                 ShaderSource &&vertex_shader,
                                 ShaderSource &&fragment_shader,
                                 sg::Camera &cam, sg::Scene &scene_)
    : Subpass{render_context, std::move(vertex_shader),
              std::move(fragment_shader)},
      camera{cam},
      scene{scene_} {}

void LightingSubpass::prepare() {
#ifndef XRY
  lighting_variant.add_definitions(
      {"MAX_LIGHT_COUNT " + std::to_string(MAX_DEFERRED_LIGHT_COUNT)});

  lighting_variant.add_definitions(light_type_definitions);
#endif
  // Build all shaders upfront
  auto &resource_cache = render_context.get_device().get_resource_cache();
  resource_cache.request_shader_module(VK_SHADER_STAGE_VERTEX_BIT,
                                       get_vertex_shader(), lighting_variant);
  resource_cache.request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT,
                                       get_fragment_shader(), lighting_variant);
}

void LightingSubpass::draw(CommandBuffer &command_buffer) {
  allocate_lights<DeferredLights>(scene.get_components<sg::Light>(),
                                  MAX_DEFERRED_LIGHT_COUNT);
  command_buffer.bind_lighting(get_lighting_state(), 0, LIGHTS_INFO_BINDING);

  // Get shaders from cache
  auto &resource_cache = command_buffer.get_device().get_resource_cache();

  auto &vert_shader_module = resource_cache.request_shader_module(
      VK_SHADER_STAGE_VERTEX_BIT, get_vertex_shader(), lighting_variant);
  auto &frag_shader_module = resource_cache.request_shader_module(
      VK_SHADER_STAGE_FRAGMENT_BIT, get_fragment_shader(), lighting_variant);

  std::vector<ShaderModule *> shader_modules{&vert_shader_module,
                                             &frag_shader_module};

  // Create pipeline layout and bind it
  auto &pipeline_layout =
      resource_cache.request_pipeline_layout(shader_modules);
  command_buffer.bind_pipeline_layout(pipeline_layout);

  // Get image views of the attachments
  auto &render_target =
      get_render_context().get_active_frame().get_render_target();
  auto &target_views = render_target.get_views();
  assert(3 < target_views.size());

  // Bind depth, albedo, and normal as input attachments
  auto &depth_view = target_views[1];
  command_buffer.bind_input(depth_view, 0, 0, 0);

  auto &albedo_view = target_views[2];
  command_buffer.bind_input(albedo_view, 0, 1, 0);

  auto &normal_view = target_views[3];
  command_buffer.bind_input(normal_view, 0, 2, 0);

  auto &metallic_roughness = target_views[4];
  command_buffer.bind_input(metallic_roughness, 0, 3, 0);

  // Set cull mode to front as full screen triangle is clock-wise
  RasterizationState rasterization_state;
  rasterization_state.cull_mode = VK_CULL_MODE_FRONT_BIT;
  command_buffer.set_rasterization_state(rasterization_state);

  // Populate uniform values
  LightUniform light_uniform;

  glm::mat4 camera_world_matrix =
      camera.get_node()->get_component<sg::Transform>().get_world_matrix();
  light_uniform.camera_pos =
      camera_world_matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  // Inverse resolution
  light_uniform.inv_resolution.x = 1.0f / render_target.get_extent().width;
  light_uniform.inv_resolution.y = 1.0f / render_target.get_extent().height;

  // Inverse view projection
  light_uniform.inv_view_proj = glm::inverse(
      vulkan_style_projection(camera.get_projection()) * camera.get_view());

  // Allocate a buffer using the buffer pool from the active frame to store
  // uniform values and bind it
  auto &render_frame = get_render_context().get_active_frame();
  auto allocation = render_frame.allocate_buffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightUniform));
  allocation.update(light_uniform);
  command_buffer.bind_buffer(allocation.get_buffer(), allocation.get_offset(),
                             allocation.get_size(), 0, GLOBAL_UNIFORM_BINDING,
                             0);

#ifdef XRY
  // Enable alpha blending
  ColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.blend_enable = VK_FALSE;

  ColorBlendState color_blend_state{};
  color_blend_state.attachments.resize(get_output_attachments().size());
  for (auto &it : color_blend_state.attachments) {
    it = color_blend_attachment;
  }
  color_blend_state.attachments[0].blend_enable = VK_FALSE;
  color_blend_state.attachments[0].alpha_blend_op = VK_BLEND_OP_ADD;
  command_buffer.set_color_blend_state(color_blend_state);

  // 光照阶段应该没有顶点输入，不需要为管线设置顶点状态，也不需要为命令缓冲绑定顶点缓冲
  VertexInputState vertex_input_state;
  command_buffer.set_vertex_input_state(vertex_input_state);

#endif

  // Draw full screen triangle triangle
  command_buffer.draw(3, 1, 0, 0);

#undef XRY
}

}  // namespace vkb