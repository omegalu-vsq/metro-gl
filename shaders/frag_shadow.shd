#version 450

in vec3 fragment_position_ws;
uniform vec3 shadow_light_position;
uniform float shadow_far_plane;

void main() {
    gl_FragDepth = length(fragment_position_ws - shadow_light_position) / shadow_far_plane;
}
