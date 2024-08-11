#version 460

precision highp int;
precision highp float;
#define MAX_LIGHT_COUNT 32

#define PI 3.1415926535

layout(input_attachment_index = 0, binding = 0) uniform subpassInput i_depth;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput i_albedo;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput i_normal;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput i_metallic_roughness;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 o_color;

layout(std140, set = 0, binding = 4) uniform GlobalUniform {
    mat4 inv_view_proj;
    vec2 inv_resolution;
	vec4 camera_pos;
} global_uniform;

struct Light {
	vec4 position;         // position.w represents type of light
	vec4 color;            // color.w represents light intensity
	vec4 direction;        // direction.w represents range
	vec2 info;             // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

layout(set = 0, binding = 5) uniform LightsInfo {
	Light directional_lights[MAX_LIGHT_COUNT];
	Light point_lights[MAX_LIGHT_COUNT];
	Light spot_lights[MAX_LIGHT_COUNT];
} lights_info;

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, float metallic, vec4 albedo) {
	vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic); // * material.specular
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}

// Specular BRDF composition --------------------------------------------
vec3 brdf(vec3 L, vec3 V, vec3 N, float metallic, float roughness, vec4 albedo) {
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);

	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	float rroughness = max(0.05, roughness);
	// D = Normal distribution (Distribution of the microfacets)
	float D = D_GGX(dotNH, roughness); 
	// G = Geometric shadowing term (Microfacets shadowing)
	float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
	// F = Fresnel factor (Reflectance depending on angle of incidence)
	vec3 F = F_Schlick(dotNV, metallic, albedo);

	return D * F * G / (4.0 * dotNL * dotNV);
}

vec3 apply_directional_light(Light light, vec3 view_dir, vec3 normal, float metallic, float roughness, vec4 albedo) {
	vec3 world_to_light = -light.direction.xyz;
	world_to_light      = normalize(world_to_light);
	float dotNL = dot(normal, world_to_light);
	float dotNV = dot(normal, view_dir);
	if(dotNL > 0.0 && dotNV > 0.0) {
		return light.color.w * dotNL * light.color.rgb * brdf(world_to_light, view_dir, normal, metallic, roughness, albedo);
	}
	return vec3(0.0);
}

vec3 apply_point_light(Light light, vec3 pos, vec3 normal) {
	vec3  world_to_light = light.position.xyz - pos;
	float dist           = length(world_to_light) * 0.005;
	float atten          = 1.0 / (dist * dist);
	world_to_light       = normalize(world_to_light);
	float ndotl          = clamp(dot(normal, world_to_light), 0.0, 1.0);
	return ndotl * light.color.w * atten * light.color.rgb;
}

vec3 apply_spot_light(Light light, vec3 pos, vec3 normal) {
	vec3  light_to_pixel   = normalize(pos - light.position.xyz);
	float theta            = dot(light_to_pixel, normalize(light.direction.xyz));
	float inner_cone_angle = light.info.x;
	float outer_cone_angle = light.info.y;
	float intensity        = (theta - outer_cone_angle) / (inner_cone_angle - outer_cone_angle);
	return smoothstep(0.0, 1.0, intensity) * light.color.w * light.color.rgb;
}

layout(constant_id = 0) const uint DIRECTIONAL_LIGHT_COUNT = 0U;
layout(constant_id = 1) const uint POINT_LIGHT_COUNT       = 0U;
layout(constant_id = 2) const uint SPOT_LIGHT_COUNT        = 0U;

void main() {
	// Retrieve position from depth
	vec4 clip = vec4(in_uv * 2.0 - 1.0, subpassLoad(i_depth).x, 1.0);
	highp vec4 world_w = global_uniform.inv_view_proj * clip;
	highp vec3 pos = world_w.xyz / world_w.w;
	vec4 albedo = subpassLoad(i_albedo);

	// Transform from [0,1] to [-1,1]
	vec3 normal = subpassLoad(i_normal).xyz;
	normal = normalize(2.0 * normal - 1.0);

    float metallic = subpassLoad(i_metallic_roughness).b;
	float roughness = subpassLoad(i_metallic_roughness).g;

	vec3 view_dir = normalize(global_uniform.camera_pos.xyz - pos);

	// Calculate lighting
	vec3 L = vec3(0.0);
	for (uint i = 0U; i < DIRECTIONAL_LIGHT_COUNT; ++i) {
		L += apply_directional_light(lights_info.directional_lights[i], view_dir, normal, metallic, roughness, albedo);
	}
	for (uint i = 0U; i < POINT_LIGHT_COUNT; ++i) {
		L += apply_point_light(lights_info.point_lights[i], pos, normal);
	}
	for (uint i = 0U; i < SPOT_LIGHT_COUNT; ++i) {
		L += apply_spot_light(lights_info.spot_lights[i], pos, normal);
	}
	vec3 ambient_color = vec3(0.3) * albedo.xyz;

	// Gamma correct
	o_color = vec4(pow(ambient_color + L * albedo.xyz, vec3(0.4545)), 1.0);
	o_color = vec4(ambient_color + L * albedo.xyz, 1.0);

	// float NV = dot(normal, view_dir);
	// if(NV<=0.0) 
	// 	o_color = vec4(1,1,1, 1.0);
	// else
	// 	o_color = vec4(0,0,0,1);
}