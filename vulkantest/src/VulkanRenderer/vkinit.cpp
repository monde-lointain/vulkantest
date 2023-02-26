#include "vkinit.h"

VkCommandPoolCreateInfo
vkinit::command_pool_create_info(
    uint32_t queue_family_index,
    VkCommandPoolCreateFlags flags
)
{
    VkCommandPoolCreateInfo command_pool = {};
    command_pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool.pNext = nullptr;
    command_pool.flags = flags;
    command_pool.queueFamilyIndex = queue_family_index;
    return command_pool;
}

VkCommandBufferAllocateInfo
vkinit::command_buffer_allocate_info(
    VkCommandPool pool,
    uint32_t count,
    VkCommandBufferLevel level
)
{
    VkCommandBufferAllocateInfo command_buffer = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = level,
        .commandBufferCount = count
    };
    return command_buffer;
}

VkPipelineShaderStageCreateInfo
vkinit::shader_stage_create_info(
    VkShaderStageFlagBits stage,
    VkShaderModule module
)
{
    VkPipelineShaderStageCreateInfo shader_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stage,
        .module = module,
        .pName = "main"
    };
    return shader_stage;
}

VkPipelineVertexInputStateCreateInfo
vkinit::vertex_input_state_create_info(VertexInputDescription& description)
{
    const VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .vertexBindingDescriptionCount = (uint32_t)description.bindings.size(),
        .pVertexBindingDescriptions = description.bindings.data(),
        .vertexAttributeDescriptionCount = (uint32_t)description.attributes.size(),
        .pVertexAttributeDescriptions = description.attributes.data()
    };
    return vertex_input;
}

VkPipelineInputAssemblyStateCreateInfo
vkinit::input_assembly_create_info(VkPrimitiveTopology topology)
{
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .topology = topology,
        .primitiveRestartEnable = VK_FALSE
    };
    return input_assembly;
}

VkPipelineRasterizationStateCreateInfo
vkinit::rasterization_state_create_info(VkPolygonMode polygon_mode)
{
    VkPipelineRasterizationStateCreateInfo raster = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = polygon_mode,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };
    return raster;
}

VkPipelineMultisampleStateCreateInfo
vkinit::multisample_state_create_info()
{
    VkPipelineMultisampleStateCreateInfo multisample = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT, // no multisample
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };
    return multisample;
}

VkPipelineColorBlendAttachmentState
vkinit::color_blend_attachment_state()
{
    VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };
    return blend_attachment;
}

VkPipelineLayoutCreateInfo
vkinit::pipeline_layout_create_info()
{
    const VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    return layout_info;
}

VkVertexInputAttributeDescription
vkinit::vertex_input_attribute_description(
    uint32_t location,
    uint32_t binding,
    VkFormat format,
    uint32_t offset
)
{
    const VkVertexInputAttributeDescription vertex_input_attribute = {
        .location = location,
        .binding = binding,
        .format = format,
        .offset = offset
    };
    return vertex_input_attribute;
}
