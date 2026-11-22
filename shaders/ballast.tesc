#version 450

layout(vertices=4) out;

in vec3 normal_control[];
in vec2 uv_control[];

out vec3 normal_eval[];
out vec2 uv_eval[];

// haute valeur qui n'est pas prise en compte sur tous les gpu
uniform float tessellation_level = 1024.0;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    normal_eval[gl_InvocationID] = normal_control[gl_InvocationID];
    uv_eval[gl_InvocationID] = uv_control[gl_InvocationID];

    if (gl_InvocationID == 0) {
        float level = min(tessellation_level, float(gl_MaxTessGenLevel));
        float level_u = abs(gl_in[0].gl_Position.z - gl_in[1].gl_Position.z) > 1.0 ? level : level / 16.0;
        float level_v = abs(gl_in[0].gl_Position.z - gl_in[3].gl_Position.z) > 1.0 ? level : level / 16.0;

        gl_TessLevelOuter[0] = level_v;
        gl_TessLevelOuter[1] = level_u;
        gl_TessLevelOuter[2] = level_v;
        gl_TessLevelOuter[3] = level_u;

        // on split surtout les faces du haut de la dalle
        gl_TessLevelInner[0] = normal_control[0].y > 0.0 ? level_u : 1.0;
        gl_TessLevelInner[1] = normal_control[0].y > 0.0 ? level_v : 1.0;
    }
}
