#pragma once

#include <vulkan/vulkan_core.h>

#include "../Model/Vertex.h"

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

    VkFenceCreateInfo
    fence_create_info(VkFenceCreateFlags flags = 0);

    VkPipelineShaderStageCreateInfo
    shader_stage_create_info(
        VkShaderStageFlagBits stage, 
        VkShaderModule module
    );

    VkPipelineVertexInputStateCreateInfo
    vertex_input_state_create_info(VertexInputDescription& description);

    VkPipelineInputAssemblyStateCreateInfo
    input_assembly_create_info(VkPrimitiveTopology topology);

    VkPipelineRasterizationStateCreateInfo
    rasterization_state_create_info(VkPolygonMode polygon_mode);

    VkPipelineMultisampleStateCreateInfo
    multisample_state_create_info();

    VkPipelineColorBlendAttachmentState
    color_blend_attachment_state();

    VkPipelineLayoutCreateInfo
    pipeline_layout_create_info();

    VkVertexInputAttributeDescription
    vertex_input_attribute_description(
        uint32_t location, 
        uint32_t binding, 
        VkFormat format, 
        uint32_t offset
    );

    VkImageCreateInfo
    image_create_info(
        VkFormat format, 
        VkImageUsageFlags usage, 
        VkExtent3D extent
    );

    VkImageViewCreateInfo
    imageview_create_info(
        VkFormat format,
        VkImage image,
        VkImageAspectFlags aspectFlags
    );

    VkPipelineDepthStencilStateCreateInfo
    depth_stencil_create_info(
        bool should_depth_test,
        bool should_depth_write,
        VkCompareOp compare_op
    );

    VkDescriptorSetLayoutBinding
    descriptorset_layout_binding(
        VkDescriptorType type,
        VkShaderStageFlags flags,
        uint32_t binding
    );

    VkWriteDescriptorSet
    write_descriptor_buffer(
        VkDescriptorType type,
        VkDescriptorSet set, 
        VkDescriptorBufferInfo* buffer_info,
        uint32_t binding
    );

    VkCommandBufferBeginInfo
    command_buffer_begin_info(VkCommandBufferUsageFlags flags);

    VkSubmitInfo vkinit::submit_info(VkCommandBuffer* command_buffer);
};	  // namespace vkinit