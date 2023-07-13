#version 460

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 frag_offset;

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection_mat;
    mat4 view_mat;
    vec4 ambient_light;
    vec3 directional_light;
    vec3 point_light_pos;
    vec4 point_light_color;
} ubo;

const float LIGHT_RADIUS = 0.1;

void main() {
    frag_offset = OFFSETS[gl_VertexIndex];

    vec4 light_cam_space = ubo.view_mat * vec4(ubo.point_light_pos, 1.0);
    vec4 pos_cam_space = light_cam_space + LIGHT_RADIUS * vec4(frag_offset, 0.0, 0.0);

    gl_Position = ubo.projection_mat * pos_cam_space;
}