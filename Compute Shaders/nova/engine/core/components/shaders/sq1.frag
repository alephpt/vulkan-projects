#version 450

layout(location = 0) in vec3 frag_color;

layout(location = 0) out vec4 out_color;

void main() {
    vec2 coordinate = gl_PointCoord - vec2(0.5);
    float distance = length(coordinate);
    float alpha = 1.0 - distance;
    out_color = vec4(frag_color, alpha);
}