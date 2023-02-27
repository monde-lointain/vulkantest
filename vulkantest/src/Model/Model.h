#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Vertex.h"
#include "../VulkanRenderer/vktypes.h"

struct Model
{
    std::vector<Vertex> vertices;
    Buffer vertex_buffer;

    bool load_from_obj(const char* filename);
};

std::unique_ptr<Model> create_model(const char* filename);