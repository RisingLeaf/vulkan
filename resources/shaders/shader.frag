#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2DArray image;

void main() {
    outColor = texture(image, vec3(texCoord, 0));
}