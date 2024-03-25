#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} UBO;

layout(location = 0) in vec3 vertpos;
layout(location = 1) in vec3 vertcol;
layout (location = 2) in vec2 textcoord;

layout (location = 0) out vec3 fragcolor;
layout (location = 1) out vec2 fragtexco;

void main() {
    gl_Position = UBO.projection * UBO.view * UBO.model * vec4(vertpos, 1.0);
    fragcolor = vertcol;
    fragtexco = textcoord;
}
