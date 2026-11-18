#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=12) out;

in vec3 shadow_position_ws[];
out vec3 fragment_position_ws;
uniform mat4 shadow_matrices[6];
uniform int shadow_wall_face;
uniform int shadow_base_layer;

void main() {
    for (int face = 0; face < 6; face++) {
        if (face == 2 || face == shadow_wall_face) continue;
        for (int vertex = 0; vertex < 3; vertex++) {
            gl_Layer = shadow_base_layer + face;
            fragment_position_ws = shadow_position_ws[vertex];
            gl_Position = shadow_matrices[face] * vec4(fragment_position_ws, 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}
