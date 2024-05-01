#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_uv;

layout(location = 0) out vec4 final_color;

layout(binding = 1) uniform sampler2D tex;

void main() {
    final_color = vec4(frag_color * texture(tex, frag_uv * 2.5 * .66 - .3).rgb, 1.0);
}