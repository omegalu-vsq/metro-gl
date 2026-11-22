#version 450

in vec2 uv_frag;

layout(binding = 0) uniform sampler2D screen_texture;
layout(location = 0) out vec4 output_color;

void main() {
    output_color = texture(screen_texture, uv_frag);
}
