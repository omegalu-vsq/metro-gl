#version 450

layout(location=0) in vec3 position;
uniform vec3 model_offset;
out vec3 shadow_position_ws;

void main() {
    shadow_position_ws = position + model_offset;
    gl_Position = vec4(shadow_position_ws, 1.0);
}
