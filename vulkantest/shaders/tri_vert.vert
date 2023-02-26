#version 460
void main()
{
    const vec3 vertices[3] = {
        vec3(-0.5f,  0.5f, 0.0f), // bottom left
        vec3( 0.5f,  0.5f, 0.0f), // bottom right
        vec3( 0.0f, -0.5f, 0.0f)  // top
    };
    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}