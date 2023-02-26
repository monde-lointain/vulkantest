#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

struct VertexInputDescription
{
	std::vector<VkVertexInputBindingDescription> bindings = {};
	std::vector<VkVertexInputAttributeDescription> attributes = {};
	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

VertexInputDescription get_vertex_input_description();