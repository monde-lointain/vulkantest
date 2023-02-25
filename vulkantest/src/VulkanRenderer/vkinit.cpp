#include "vkinit.h"

VkCommandPoolCreateInfo vkinit::command_pool_create_info(
	uint32_t queue_family_index, 
	VkCommandPoolCreateFlags flags
)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	info.queueFamilyIndex = queue_family_index;
	return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(
	VkCommandPool pool, 
	uint32_t count, 
	VkCommandBufferLevel level
)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;
	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}
