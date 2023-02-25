#pragma once

#include <vulkan/vulkan_core.h>

namespace vkinit
{
    VkCommandPoolCreateInfo
    command_pool_create_info(
        uint32_t queue_family_index = 0, 
        VkCommandPoolCreateFlags flags = 0
    );

    VkCommandBufferAllocateInfo
    command_buffer_allocate_info(
        VkCommandPool pool,
        uint32_t count = 1,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
    );

    VkPipelineShaderStageCreateInfo
    shader_stage_create_info(
        VkShaderStageFlagBits stage, 
        VkShaderModule module
    );

    VkPipelineVertexInputStateCreateInfo
    vertex_input_state_create_info();

    VkPipelineInputAssemblyStateCreateInfo
    input_assembly_create_info(VkPrimitiveTopology topology);

    VkPipelineRasterizationStateCreateInfo
    rasterization_state_create_info(VkPolygonMode polygon_mode);

    VkPipelineMultisampleStateCreateInfo
    multisample_state_create_info();

    VkPipelineColorBlendAttachmentState
    color_blend_attachment_state();
};	  // namespace vkinit