#include "Model.h"

#define FAST_OBJ_IMPLEMENTATION

#include <iostream>

#include <fast_obj/fast_obj.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../Utils/Colors.h"
#include "../Utils/string_ops.h"

bool Model::load_from_obj(const char* filename)
{
    fastObjMesh* fast_mesh = fast_obj_read(filename);

    if (!fast_mesh)
    {
        std::cerr << "Failed to load " << filename << ".\n";
        return false;
    }

    Vertex vertex = {};

    // For each mesh
    for (uint32_t i = 0; i < fast_mesh->object_count; i++)
    {
        const fastObjGroup object = fast_mesh->objects[i];
        int idx = 0;

        // For each triangle
        for (uint32_t j = 0; j < object.face_count; j++)
        {
            // For each vertex
            for (uint32_t k = 0; k < 3; k++)
            {
                const fastObjIndex index = fast_mesh->indices[object.index_offset + idx];

                vertex.position = glm::vec3(
                    fast_mesh->positions[3 * index.p + 0],
                    fast_mesh->positions[3 * index.p + 1],
                    fast_mesh->positions[3 * index.p + 2]
                );
                vertex.texcoord = glm::vec2(
                    fast_mesh->texcoords[2 * index.t + 0],
                    fast_mesh->texcoords[2 * index.t + 1]
                );
                vertex.normal = glm::vec3(
                    fast_mesh->normals[3 * index.n + 0],
                    fast_mesh->normals[3 * index.n + 1],
                    fast_mesh->normals[3 * index.n + 2]
                );
                vertex.color = Colors::WHITE;

                // Add new vertex to the moel
                vertices.push_back(vertex);

                idx++;
            }
        }
    }

    std::cout << "Loaded " << filename << "\n";

    // Destroy the fastobj mesh once we've imported it
    fast_obj_destroy(fast_mesh);

    return true;
}

void Model::update()
{
    const glm::mat4 translation_matrix =
        glm::translate(glm::mat4(1.0f), translation);

    const glm::mat4 rotation_x_matrix =
        glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    const glm::mat4 rotation_y_matrix =
        glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::mat4 rotation_z_matrix =
        glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

    const glm::mat4 rotation_matrix =
        rotation_x_matrix * rotation_y_matrix * rotation_z_matrix;

    transform = translation_matrix * rotation_matrix * scale_matrix;
}

std::shared_ptr<Model> create_model(
    const char* filename,
    const glm::vec3& rotation,
    const glm::vec3& scale,
    const glm::vec3& translation
)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();
    const bool success = model->load_from_obj(filename);

    if (!success)
    {
        return nullptr;
    }

    model->rotation = rotation;
    model->scale = scale;
    model->translation = translation;

    return model;
}
