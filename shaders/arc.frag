#version 450

uniform vec3 color;
uniform float anim_time;

layout(location=0) out vec4 output_color;
layout(location=1) out vec4 output_emission;

void main() {
    output_color = vec4(color, 1.0 - anim_time);
    vec3 linear_color = mix(pow((color + 0.055) / 1.055, vec3(2.4)), color / 12.92, lessThanEqual(color, vec3(0.04045)));
    output_emission = vec4(linear_color * 2.0, output_color.a);
}
