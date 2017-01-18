#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec4 mouse;
    vec2 resolution;
    float time;
} constants;

vec4 mouse = constants.mouse;
vec2 resolution = constants.resolution;
float time = constants.time;

void main() {
    outColor = vec4(mouse.xy / resolution.xy, 0.0, 0.0);
    outColor = vec4(pos.xy, 0.0, 0.0);
}
