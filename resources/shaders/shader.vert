#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 fragtexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
    vec2 offset;
    vec3 color_diff;
} ubo;

void main() {
    gl_Position = vec4(position + ubo.offset, 0.0, 1.0);
    fragtexCoord = texCoord;
}