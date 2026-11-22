#version 450

layout(quads, equal_spacing, ccw) in;

in vec3 normal_eval[];
in vec2 uv_eval[];

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 model_offset;
uniform vec3 camera_ws;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 camera_pos;
out vec2 uv_frag;

float relief_height = 0.3;
float relief_scale = 7.0;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(13.0, 79.0))) * 10000.0);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float ballast_height(vec2 p) {
    float relief = noise(p * relief_scale);

    relief += 0.5 * noise(p * relief_scale * 2.7);
    relief = relief / 1.5;

    return (relief - 0.5) * relief_height;
}

void main() {
    vec2 coords = gl_TessCoord.xy;
    vec4 weights = vec4((1.0 - coords.x) * (1.0 - coords.y), coords.x * (1.0 - coords.y), coords.x * coords.y, (1.0 - coords.x) * coords.y);

    vec3 object_position = weights.x * gl_in[0].gl_Position.xyz + weights.y * gl_in[1].gl_Position.xyz
                         + weights.z * gl_in[2].gl_Position.xyz + weights.w * gl_in[3].gl_Position.xyz;

    normal_ws = normalize(weights.x * normal_eval[0] + weights.y * normal_eval[1] + weights.z * normal_eval[2] + weights.w * normal_eval[3]);

    uv_frag = weights.x * uv_eval[0] + weights.y * uv_eval[1] + weights.z * uv_eval[2] + weights.w * uv_eval[3];

    object_position += model_offset;
    object_position.y += ballast_height(object_position.xz);

    position_ws = object_position;
    camera_pos = camera_ws;

    gl_Position = projection_matrix * view_matrix * vec4(object_position, 1.0);
}
