#include "vkutils.h"

#include <iostream>

void check_vk_result(VkResult err)
{
	if (err == 0)
	{
		return;
	}
	std::cerr << "[vulkan] Error: VkResult = " << err << "\n";
	if (err < 0)
	{
		abort();
	}
}