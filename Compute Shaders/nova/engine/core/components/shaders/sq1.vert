#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec3 frag_color;

void main()
{
    gl_PointSize = 10.0;
    gl_Position = vec4(position, 1.0, 1.0);
    frag_color = color.rgb;
}