#version 450

precision highp float;

#define PI 3.1415926535

#ifdef HAS_BASE_COLOR_TEXTURE
layout (set=0, binding=0) uniform sampler2D base_color_texture;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
layout (set=0, binding=2) uniform sampler2D metallic_roughness_texture;
#endif

#ifdef HAS_NORMAL_TEXTURE
layout (set=0, binding=3) uniform sampler2D normal_texture;
#endif

#ifdef HAS_OCCLUSION_TEXTURE
layout (set=0, binding=4) uniform sampler2D occlusion_texture;
#endif

#ifdef HAS_EMISSIVE_TEXTURE
layout (set=0, binding=5) uniform sampler2D emissive_texture;
#endif

layout (location = 0) in vec4 in_pos;

#ifdef HAS_NORMAL
layout (location = 1) in vec3 in_normal;
#endif

#ifdef HAS_TEXCOORD_0
layout (location = 2) in vec2 in_uv;
#endif

#ifdef HAS_TANGENT
layout (location = 3) in vec3 in_tangent;
#endif

layout (location = 0) out vec4 o_albedo;

layout (location = 1) out vec4 o_normal;

layout (location = 2) out vec4 o_mro;

layout(set = 0, binding = 1) uniform GlobalUniform {
    mat4 model;
    mat4 view_proj;
    vec3 camera_position;
} global_uniform;

layout(push_constant, std430) uniform PBRMaterialUniform {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
} pbr_material_uniform;

void main() {

// 法线
#if defined(HAS_NORMAL) && defined(HAS_TANGENT) && defined(HAS_NORMAL_TEXTURE) && defined(HAS_TEXCOORD_0)
    vec3 normal = normalize(in_normal);
    vec3 tangent = normalize(in_tangent);
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, normal);
    vec3 tbn_normal = texture(normal_texture, in_uv).rgb * 2 - 1;
    o_normal = vec4(0.5 * normalize(TBN * tbn_normal) + 0.5, 1.0);
#elif defined(HAS_NORMAL)
    vec3 normal = normalize(in_normal);
    // Transform normals from [-1, 1] to [0, 1]
    o_normal = vec4(0.5 * normal + 0.5, 1.0);
#else
    o_normal = vec4(1, 1, 1, 1);
#endif

    vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);

#if defined(HAS_BASE_COLOR_TEXTURE) && defined(HAS_TEXCOORD_0)
    base_color = texture(base_color_texture, in_uv);
#else
    base_color = pbr_material_uniform.base_color_factor;
#endif

    o_albedo = vec4(base_color);

#if defined(HAS_EMISSIVE_TEXTURE) && defined(HAS_TEXCOORD_0)
    o_albedo = o_albedo + texture(emissive_texture, in_uv);
#endif

#if defined(HAS_METALLIC_ROUGHNESS_TEXTURE) && defined(HAS_TEXCOORD_0)
    o_mro = texture(metallic_roughness_texture, in_uv);
#else
    o_mro = vec4(pbr_material_uniform.metallic_factor, pbr_material_uniform.roughness_factor, 0.0, 0.0);
#endif
}
