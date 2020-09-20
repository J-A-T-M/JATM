#version 330 core
#define NUM_LIGHTS 2

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec2 v_texCoord;
out vec3 v_position;
out vec4 v_lightSpace_position[NUM_LIGHTS];
out vec3 v_normal;
out mat3 v_tangent2view;

uniform mat3 modelNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpace[NUM_LIGHTS];

void main() {
	v_texCoord = texCoord;

	vec4 world_space_position = model * vec4(position, 1.0);
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		v_lightSpace_position[i] = lightSpace[i] * world_space_position;
	}

	vec3 T = normalize(mat3(view) * modelNormal * tangent);
	vec3 N = normalize(mat3(view) * modelNormal * normal);
	vec3 B = normalize(mat3(view) * modelNormal * bitangent);
	v_normal = N;
	v_tangent2view = mat3(T, B, N);

	vec4 view_space_position = view * world_space_position;
	v_position = view_space_position.xyz;
    gl_Position = projection * view_space_position;
}