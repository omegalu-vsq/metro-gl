#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

out vec3 normal_control;
out vec2 uv_control;

void main() {
    normal_control = normal;
    uv_control = uv;

    gl_Position = vec4(position, 1.0);
}
