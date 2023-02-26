#version 460

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 fcolor;

void main()
{
    fcolor = vec4(color, 1.0f);
}