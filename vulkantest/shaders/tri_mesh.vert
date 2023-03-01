#version 460

// Vertex shader inputs
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 color;

layout(set = 0, binding = 0) uniform CameraMatrices
{
    mat4 vp_matrix;
} camera_data;

struct ObjectData
{
    mat4 transform;
};

// Transforms of the models coming in
layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
    ObjectData objects[];
} object_buffer;

layout(location = 0) out vec3 out_color;

void main()
{
    // Concatenate model and VP matrices into MVP matrix
    mat4 model_matrix = object_buffer.objects[gl_BaseInstance].transform;
    mat4 modelviewprojection = (camera_data.vp_matrix * model_matrix);

    // Transform vertex from local space to clip space with MVP matrix
    gl_Position = modelviewprojection * vec4(position, 1.0f);
    out_color = color;
}