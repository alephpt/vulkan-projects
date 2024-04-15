#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 iColor;

layout(location = 0) out vec3 oColor;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0);
    oColor = iColor;
}
