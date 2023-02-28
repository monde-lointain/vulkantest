#pragma once

#include <glm/vec4.hpp>

/** Contains data for a scene */
struct Scene
{
	glm::vec4 fog_color;          // w = exponent
	glm::vec4 fog_distance;       // x = min, y = max, z/w = unused
	glm::vec4 ambient_color;
	glm::vec4 sunlight_direction; // w = intensity
	glm::vec4 sunlight_color;
};

