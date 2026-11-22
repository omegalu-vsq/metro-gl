#version 450

out vec2 uv_frag;

const vec2 points[] = vec2[](
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0)
);

void main() {
    vec2 position = points[gl_VertexID];
    uv_frag = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
