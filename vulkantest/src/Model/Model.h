#pragma once

#include <array>
#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Vertex.h"
#include "../VulkanRenderer/vktypes.h"

struct Material
{
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
};

struct Model
{
    std::vector<Vertex> vertices;
    Buffer vertex_buffer;
    std::unique_ptr<Material> material;

    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 translation = glm::vec3(0.0f);
    glm::mat4 transform;

    bool load_from_obj(const char *filename);
    void update();
};

std::unique_ptr<Model> create_model(
    const char* filename,
    const glm::vec3& rotation = glm::vec3(0.0f),
    const glm::vec3& scale = glm::vec3(1.0f),
    const glm::vec3& translation = glm::vec3(0.0f)
);