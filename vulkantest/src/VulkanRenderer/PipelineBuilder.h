#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>

/** Creates a pipeline layout for resources */
struct PipelineBuilder
{
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo raster;
    VkPipelineColorBlendAttachmentState blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineLayout pipeline_layout;

    VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};