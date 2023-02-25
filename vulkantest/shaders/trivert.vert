#version 460

void main()
{
    vec3 vertices[3] = {
        vec3(-0.5f, -0.5f, 0.0f),
        vec3( 0.5f, -0.5f, 0.0f),
        vec3( 0.0f,  0.5f, 0.0f)
    };

    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}