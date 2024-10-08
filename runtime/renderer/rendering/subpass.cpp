#include "subpass.h"
namespace vkb {

glm::mat4 vulkan_style_projection(const glm::mat4 &proj) {
  // Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
  glm::mat4 mat = proj;
  mat[1][1] *= -1;
  return mat;
}

const std::vector<std::string> light_type_definitions = {
    "DIRECTIONAL_LIGHT " +
        std::to_string(static_cast<float>(sg::LightType::Directional)),
    "POINT_LIGHT " + std::to_string(static_cast<float>(sg::LightType::Point)),
    "SPOT_LIGHT " + std::to_string(static_cast<float>(sg::LightType::Spot))};

Subpass::Subpass(RenderContext &render_context, ShaderSource &&vertex_source,
                 ShaderSource &&fragment_source)
    : render_context{render_context},
      vertex_shader{std::move(vertex_source)},
      fragment_shader{std::move(fragment_source)} {}

void Subpass::update_render_target_attachments(RenderTarget &render_target) {
  render_target.set_input_attachments(input_attachments);
  render_target.set_output_attachments(output_attachments);
}

RenderContext &Subpass::get_render_context() { return render_context; }

const ShaderSource &Subpass::get_vertex_shader() const { return vertex_shader; }

const ShaderSource &Subpass::get_fragment_shader() const {
  return fragment_shader;
}

DepthStencilState &Subpass::get_depth_stencil_state() {
  return depth_stencil_state;
}

const std::vector<uint32_t> &Subpass::get_input_attachments() const {
  return input_attachments;
}

void Subpass::set_input_attachments(std::vector<uint32_t> input) {
  input_attachments = input;
}

const std::vector<uint32_t> &Subpass::get_output_attachments() const {
  return output_attachments;
}

void Subpass::set_output_attachments(std::vector<uint32_t> output) {
  output_attachments = output;
}

const std::vector<uint32_t> &Subpass::get_color_resolve_attachments() const {
  return color_resolve_attachments;
}

void Subpass::set_color_resolve_attachments(
    std::vector<uint32_t> color_resolve) {
  color_resolve_attachments = color_resolve;
}

const bool &Subpass::get_disable_depth_stencil_attachment() const {
  return disable_depth_stencil_attachment;
}

void Subpass::set_disable_depth_stencil_attachment(bool disable_depth_stencil) {
  disable_depth_stencil_attachment = disable_depth_stencil;
}

const uint32_t &Subpass::get_depth_stencil_resolve_attachment() const {
  return depth_stencil_resolve_attachment;
}

void Subpass::set_depth_stencil_resolve_attachment(
    uint32_t depth_stencil_resolve) {
  depth_stencil_resolve_attachment = depth_stencil_resolve;
}

const VkResolveModeFlagBits Subpass::get_depth_stencil_resolve_mode() const {
  return depth_stencil_resolve_mode;
}

void Subpass::set_depth_stencil_resolve_mode(VkResolveModeFlagBits mode) {
  depth_stencil_resolve_mode = mode;
}

LightingState &Subpass::get_lighting_state() { return lighting_state; }

const std::string &Subpass::get_debug_name() const { return debug_name; }

void Subpass::set_debug_name(const std::string &name) { debug_name = name; }

}  // namespace vkb