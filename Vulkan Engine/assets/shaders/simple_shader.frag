#version 460

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_pos_world;
layout (location = 2) in vec3 frag_norm_world;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection_view_mat;
    vec4 ambient_light;
    vec3 directional_light;
    vec3 point_light_pos;
    vec4 point_light_color;
} ubo;

layout (push_constant) uniform Push {
    mat4 model_mat;
    mat4 normal_mat;
} push;

void main() {
    vec3 light_direction = ubo.point_light_pos - frag_pos_world;

    float attenuation = 1.0 / dot(light_direction, light_direction);

    vec3 light_color = ubo.point_light_color.xyz * ubo.point_light_color.w * attenuation;

    vec3 ambient_light = ubo.ambient_light.xyz * ubo.ambient_light.w;

    vec3 diffuse_light = light_color * max(dot(normalize(frag_norm_world), light_direction), 0.0);

    out_color = vec4((diffuse_light + ambient_light) * frag_color, 1.0);
}