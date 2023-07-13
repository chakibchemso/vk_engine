#version 460

layout (location = 0) in vec2 frag_offset;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection_mat;
    mat4 view_mat;
    vec4 ambient_light;
    vec3 directional_light;
    vec3 point_light_pos;
    vec4 point_light_color;
} ubo;

void main() {
    float dist = sqrt(dot(frag_offset, frag_offset));
    if (dist >= 1.0) discard;

    out_color = vec4(ubo.point_light_color.xyz, 1.0);
}