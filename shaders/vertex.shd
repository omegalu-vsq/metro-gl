#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 model_offset;

uniform vec3 camera_ws;

out vec3 position_view;
out vec3 position_world;
out vec3 normal_frag;
out vec3 object_normal_frag;
out vec2 uv_frag;
out vec3 object_pos_frag;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 camera_pos;

out vec3 light_color;

void main() {
	vec3 object_position = position + model_offset;
	vec4 pos = view_matrix * vec4(object_position, 1.0);

	position_world = object_position;

	position_view = pos.xyz;
	normal_frag = mat3(view_matrix) * normal;
	//normal_frag = (view_matrix * vec4(normal, 1.0)).xyz;
	object_normal_frag = normal;
	uv_frag = uv;
	object_pos_frag = object_position;

	position_ws = object_position;
	normal_ws = normal;
	camera_pos = camera_ws;

	gl_Position = projection_matrix * pos;
}
