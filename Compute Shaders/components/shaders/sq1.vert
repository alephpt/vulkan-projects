#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 iColor;

layout(location = 0) out vec3 oColor;

void main()
{
    gl_Position = vec4(position, 1.0);
    oColor = iColor;
}
