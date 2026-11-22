#version 450

uniform vec3 color;

layout(location=0) out vec4 output_color;
layout(location=1) out vec4 output_emission;

void main() {
    output_emission = vec4(0.0, 0.0, 0.0, 1.0);

    output_color = vec4(color, 1.0);
}
