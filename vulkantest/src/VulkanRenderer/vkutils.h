#pragma once

#include <format>
#include <iostream>

#include <vulkan/vulkan_core.h>

inline constexpr
void VK_CHECK(VkResult result)
{
    if (result != VK_SUCCESS)
    {
        std::cout << std::format("[vulkan] Error: {}\n", (int)result);
        if (result < 0)
        {
            abort();
        }
    }
}