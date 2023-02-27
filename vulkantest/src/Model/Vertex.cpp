#include "Vertex.h"

#include "../VulkanRenderer/vkinit.h"

VertexInputDescription get_vertex_input_description()
{
    VertexInputDescription description;

    // One binding with addressing done per instance
    const VkVertexInputBindingDescription input_binding = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    description.bindings.push_back(input_binding);

    // Define vertex attributes for position, normal and color
    const uint32_t pos_offset = offsetof(Vertex, position);
    const uint32_t texcoord_offset = offsetof(Vertex, texcoord);
    const uint32_t normal_offset = offsetof(Vertex, normal);
    const uint32_t color_offset = offsetof(Vertex, color);

    // Set position to be location 0
    const VkVertexInputAttributeDescription pos_attr = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = pos_offset
    };

    // Set texture coordinate to be location 1
    const VkVertexInputAttributeDescription texcoord_attr = {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = texcoord_offset
    };

    // Set normal to be location 2
    const VkVertexInputAttributeDescription normal_attr = {
        .location = 2,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = normal_offset
    };

    // Set color to be location 3
    const VkVertexInputAttributeDescription color_attr = {
        .location = 3,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = color_offset
    };

    description.attributes.push_back(pos_attr);
    description.attributes.push_back(texcoord_attr);
    description.attributes.push_back(normal_attr);
    description.attributes.push_back(color_attr);

    return description;
}
