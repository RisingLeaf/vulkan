#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    vec2 offset;
    vec3 color_diff;
} push;

void main() {
    gl_Position = vec4(position + push.offset, 0.0, 1.0);
    fragColor = color;
}