#include "Model.h"

#include <iostream>

#include <fast_obj/fast_obj.h>

#include "../Utils/Colors.h"

void Model::load_from_obj(const char* filename)
{
	fastObjMesh* fast_mesh = fast_obj_read(filename);

	if (!fast_mesh)
	{
		std::cerr << "Failed to load " << filename << ".\n";
		return;
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
}

Model* create_model(const char* filename)
{
	Model* model = new Model;
	model->load_from_obj(filename);
	if (!model)
	{
		return nullptr;
	}
	return model;
}
