#pragma once

#include <glm/vec3.hpp>

// Format R32G32B32 float
namespace Colors
{
	constexpr glm::vec3 BLACK(0.0f);
	constexpr glm::vec3 WHITE(1.0f);
	constexpr glm::vec3 RED(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 GREEN(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 BLUE(0.0f, 0.0f, 1.0f);
	constexpr glm::vec3 CYAN(0.0f, 1.0f, 1.0f);
	constexpr glm::vec3 YELLOW(1.0f, 1.0f, 0.0f);
	constexpr glm::vec3 MAGENTA(1.0f, 0.0f, 1.0f);
}