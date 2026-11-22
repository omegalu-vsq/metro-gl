#version 450

layout(vertices=2) out;

uniform int active_arc;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = gl_PrimitiveID == active_arc ? 1.0 : 0.0;
        gl_TessLevelOuter[1] = 32.0;
    }
}
