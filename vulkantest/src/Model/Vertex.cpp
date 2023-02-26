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
    const uint32_t normal_offset = offsetof(Vertex, normal);
    const uint32_t color_offset = offsetof(Vertex, color);

    const VkVertexInputAttributeDescription pos_attr = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = pos_offset
    };

    const VkVertexInputAttributeDescription normal_attr = {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = normal_offset
    };

    const VkVertexInputAttributeDescription color_attr = {
        .location = 2,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = color_offset
    };

    description.attributes.push_back(pos_attr);
    description.attributes.push_back(normal_attr);
    description.attributes.push_back(color_attr);

    return description;
}
