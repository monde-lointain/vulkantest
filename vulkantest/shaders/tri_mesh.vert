#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 texcoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 color;

layout (location = 0) out vec3 out_color;

layout(set = 0, binding = 0) uniform ubo
{
    mat4 modelviewprojection;
} mvp;

void main()
{
    gl_Position = mvp.modelviewprojection * vec4(position, 1.0f);
    out_color = color;
}