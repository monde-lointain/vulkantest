#version 460

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 out_fcolor;

layout(set = 0, binding = 1) uniform SceneData {
	vec4 fog_color;          // w = exponent
	vec4 fog_distance;       // x = min, y = max, z/w = unused
	vec4 ambient_color;
	vec4 sunlight_direction; // w = intensity
	vec4 sunlight_color;
} scene_data;

void main()
{
	out_fcolor = vec4(color + scene_data.ambient_color.xyz, 1.0f);
}