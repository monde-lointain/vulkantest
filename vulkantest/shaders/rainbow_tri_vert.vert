#version 460
layout (location = 0) out vec3 color;
void main()
{
    const vec3 vertices[3] = {
        vec3(-0.5f,  0.5f, 0.0f), // bottom left
        vec3( 0.5f,  0.5f, 0.0f), // bottom right
        vec3( 0.0f, -0.5f, 0.0f)  // top
    };
    const vec3 colors[3] = {
        vec3(1.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 0.0f, 1.0f),
    };

    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
    color = colors[gl_VertexIndex];
}