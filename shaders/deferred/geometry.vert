#version 460

layout(location = 0) in vec3 position;

#ifdef HAS_NORMAL
layout(location = 1) in vec3 normal;
#endif

#ifdef HAS_TEXCOORD_0
layout(location = 2) in vec2 texcoord_0;
#endif

#ifdef HAS_TANGENT
layout(location = 3) in vec4 tangent;
#endif

layout(set = 0, binding = 1) uniform GlobalUniform {
    mat4 model;
    mat4 view_proj;
    vec3 camera_position;
} global_uniform;

layout (location = 0) out vec4 o_pos;

#ifdef HAS_NORMAL
layout (location = 1) out vec3 o_normal;
#endif

#ifdef HAS_TEXCOORD_0
layout (location = 2) out vec2 o_uv;
#endif

#ifdef HAS_TANGENT
layout (location = 3) out vec3 o_tangent;
#endif

void main()
{
    o_pos = global_uniform.model * vec4(position, 1.0);

#ifdef HAS_TEXCOORD_0
    o_uv = texcoord_0;
#endif

#ifdef HAS_NORMAL
    o_normal = mat3(global_uniform.model) * normal;
#endif

#ifdef HAS_TANGENT
    o_tangent = mat3(global_uniform.model) * tangent.xyz;
#endif

    gl_Position = global_uniform.view_proj * o_pos;
}
