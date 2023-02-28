#include "string_ops.h"

#include <iomanip>
#include <sstream>

std::string mat4_to_string(const glm::mat4& m, int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ss << m[j][i] << "\t";
		}
		ss << "\n";
	}
	std::string result = ss.str();
	return result;
}

std::string vec3_to_string(const glm::vec3& v, int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	std::string result = ss.str();
	return result;
}

std::string vec4_to_string(const glm::vec4& v, int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	std::string result = ss.str();
	return result;
}

std::string to_string_with_commas(uint64_t n)
{
	std::stringstream ss;
	ss << n;
	std::string str = ss.str();

	int count = 0;
	for (int i = (int)str.length() - 1; i >= 0; i--)
	{
		count++;
		if (count % 3 == 0 && i != 0)
		{
			str.insert(i, ",");
		}
	}

	return str;
}
