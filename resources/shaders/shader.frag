#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec2 offset;
    vec3 color_diff;
} push;

void main() {
    outColor = fragColor * vec4(push.color_diff, 1.0);
}