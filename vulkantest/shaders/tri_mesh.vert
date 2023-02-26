#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 texcoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 color;

layout (location = 0) out vec3 out_color;

layout (push_constant) uniform push_constants
{
    vec4 data;
    mat4 modelviewprojection;
} pc;

void main()
{
    gl_Position = pc.modelviewprojection * vec4(position, 1.0f);
    out_color = color;
}