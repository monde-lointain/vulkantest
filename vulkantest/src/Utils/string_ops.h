#pragma once

#include <string>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

std::string mat4_to_string(const glm::mat4& m, int precision = 2);
std::string vec3_to_string(const glm::vec3& v, int precision = 2);
std::string vec4_to_string(const glm::vec4& v, int precision = 2);
std::string to_string_with_commas(uint64_t n);

