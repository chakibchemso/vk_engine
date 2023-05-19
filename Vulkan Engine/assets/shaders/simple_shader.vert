#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 uv;

layout (location = 0) out vec3 frag_color;

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

const float AMBIENT_LIGHT = 0.2;

void main() {
    vec4 world_pos = push.model_mat * vec4(position, 1.0);

    gl_Position = ubo.projection_view_mat * world_pos;

    vec3 normal_world_space = normalize(mat3(push.normal_mat) * normal);

    vec3 light_direction = ubo.point_light_pos - world_pos.xyz;

    float attenuation = 1.0 / dot(light_direction, light_direction);

    vec3 light_color = ubo.point_light_color.xyz * ubo.point_light_color.w * attenuation;

    vec3 ambient_light = ubo.ambient_light.xyz * ubo.ambient_light.w;

    vec3 diffuse_light = light_color * max(dot(normal_world_space, light_direction), 0.0);

    frag_color = (diffuse_light + ambient_light) * color;
}