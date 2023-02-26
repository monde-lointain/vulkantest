#pragma once

#include <vma/vk_mem_alloc.h>

/** Memory buffer managed by VMA */
struct Buffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};