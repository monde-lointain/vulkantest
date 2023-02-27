#pragma once

#include <vma/vk_mem_alloc.h>

/** Memory buffer managed by VMA */
struct Buffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct Image
{
    VkImage image;
    VmaAllocation allocation;
};