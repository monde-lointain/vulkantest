#pragma once

#include <vector>

#include "Vertex.h"
#include "../VulkanRenderer/vktypes.h"

struct Model
{
    std::vector<Vertex> vertices;
    Buffer vertex_buffer;
};

