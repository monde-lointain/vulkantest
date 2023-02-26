#pragma once

#include <array>
#include <vector>

#include "Vertex.h"
#include "../VulkanRenderer/vktypes.h"

struct Model
{
    std::vector<Vertex> vertices;
    Buffer vertex_buffer;

    void load_from_obj(const char* filename);
};

Model* create_model(const char* filename);