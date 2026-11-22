#version 450

layout(isolines, equal_spacing) in;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 model_offset;
uniform float anim_time;
uniform float arc_seed;

float arc_force = 0.20;
float noise_force = 0.10;

float hash(float p) {
    return fract(sin(p * 13.0) * 10000.0);
}

void main() {
    float t = gl_TessCoord.x;
    float envelope = sin(3.14 * t);
    float random_value = hash(t * 32.0 + arc_seed);
    float phase = random_value * 6.28;

    vec3 a = gl_in[0].gl_Position.xyz;
    vec3 b = gl_in[1].gl_Position.xyz;
    vec3 direction = normalize(b - a);
    vec3 side = normalize(cross(direction, vec3(1.0, 0.0, 0.0)));

    if (side.y < 0.0) side = -side;

    vec3 transverse = cross(direction, side);
    vec3 position = mix(a, b, t);

    float irregularity = (random_value - 0.5) * noise_force + sin(anim_time * 12.0 + phase) * 0.03;
    position += side * envelope * (arc_force + irregularity);
    position += transverse * envelope * sin(anim_time * 18.0 + phase) * 0.025;

    position += model_offset;

    gl_Position = projection_matrix * view_matrix * vec4(position, 1.0);
}
