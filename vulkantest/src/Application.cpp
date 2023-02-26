#include "Application.h"

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk-bootstrap/VkBootstrap.h>

#include "VulkanRenderer/PipelineBuilder.h"
#include "VulkanRenderer/vkinit.h"
#include "VulkanRenderer/vkutils.h"

#ifdef NDEBUG
const bool VALIDATION_LAYERS_ON = false;
#else
const bool VALIDATION_LAYERS_ON = true;
#endif

void Application::setup()
{
}

void Application::input()
{
}

void Application::update()
{
    SDL_Event event;

    // Handling core SDL events (moving the mouse, closing the window, etc.)
    while (SDL_PollEvent(&event))
    {
        // Closing the window
        switch (event.type)
        {
            case SDL_QUIT:
            {
                running = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                    break;
                }
            }
        }
    }
}

constexpr uint64_t ONE_SECOND = 1000000000;
constexpr uint64_t TIMEOUT_PERIOD = ONE_SECOND;

void Application::render()
{
    // Wait until the GPU has finished rendering, looping in case it takes
    // longer than expected
    VkResult result;
    do
    {
        result = vkWaitForFences(
            device, 
            1, 
            &render_fence, 
            true, 
            TIMEOUT_PERIOD
        );
    } while (result == VK_TIMEOUT);
    VK_CHECK(vkResetFences(device, 1, &render_fence));

    // Request an image from swapchain
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(
        device, 
        swapchain, 
        TIMEOUT_PERIOD,
        present_semaphore, 
        nullptr, 
        &swapchain_image_index)
    );

    // Clear the current command buffer
    VK_CHECK(vkResetCommandBuffer(main_command_buffer, 0));

    // Start recording commands into the command buffer
    const VkCommandBufferBeginInfo cmd_buf_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };
    VK_CHECK(vkBeginCommandBuffer(main_command_buffer, &cmd_buf_begin_info));

    // Set draw color for clearing the screen
    VkClearValue clear_value;
    //float flash = fabsf(sinf((float)current_frame / 120.f));
    clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    // Begin a render pass
    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass,
        .framebuffer = framebuffers[swapchain_image_index],
        .renderArea = { .offset = { 0, 0 }, .extent = window_extent },
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };
    vkCmdBeginRenderPass(
        main_command_buffer, 
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // Draw a triangle to the screen
	vkCmdBindPipeline(
		main_command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline
    );
    vkCmdDraw(main_command_buffer, 3, 1, 0, 0);

    // Finalize render stage commands
    vkCmdEndRenderPass(main_command_buffer);
    VK_CHECK(vkEndCommandBuffer(main_command_buffer));

    // Submit command buffer to the graphics queue
    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &present_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &main_command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_semaphore
    };
    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, render_fence));

    // Present image to the swap chain
    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &swapchain_image_index
    };
    VK_CHECK(vkQueuePresentKHR(queue, &present_info));
}

void Application::initialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            window_name,
            "Failed to initialize SDL.",
            nullptr
        );
        return;
    }

    // Create a window
    window = SDL_CreateWindow(
        window_name,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        (int)window_extent.width, (int)window_extent.height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS
    );
    if (!window)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            window_name,
            "Failed to initialize SDL window.",
            nullptr
        );
        SDL_Quit();
        return;
    }

    // Initialize the Vulkan renderer
    init_instance();
    init_swapchain();
    init_default_renderpass();
    init_framebuffers();
    init_commands();
    //init_descriptors();
    init_sync_objects();
    init_pipelines();

    // Everything is successfully initialized and the application is running
    running = true;
}

void Application::run()
{
    setup();

    while (running)
    {
        input();
        update();
        render();
        
        current_frame++;
    }
}

void Application::destroy() const
{
    // Free Vulkan resources
    vkDestroySemaphore(device, present_semaphore, nullptr);
    vkDestroySemaphore(device, render_semaphore, nullptr);
    vkDestroyFence(device, render_fence, nullptr);
    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchain_image_views[i], nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkb::destroy_debug_utils_messenger(instance, debug_messenger);
    vkDestroyInstance(instance, nullptr);

    // Free SDL resources
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::init_instance()
{
    // Create a Vulkan instance and debug messenger
    vkb::InstanceBuilder builder;
    builder.set_app_name("Vulkan Renderer");
    builder.require_api_version(1, 1, 0);
    if (VALIDATION_LAYERS_ON)
    {
        builder.request_validation_layers(true); // Enables "VK_LAYER_KHRONOS_validation"
        builder.enable_layer("VK_LAYER_LUNARG_api_dump");
        builder.use_default_debug_messenger();
    }
    const vkb::Instance vkb_inst = builder.build().value();

    instance = vkb_inst.instance;
    debug_messenger = vkb_inst.debug_messenger;

    // Create a Vulkan surface to draw to
    SDL_Vulkan_CreateSurface(window, instance, &surface);

    // Select a GPU from the available physical devices
    vkb::PhysicalDeviceSelector selector(vkb_inst);
    selector.set_minimum_version(1, 1);
    selector.set_surface(surface);
    const vkb::PhysicalDevice vkb_gpu = selector.select().value();

    gpu = vkb_gpu.physical_device;

    // Create a Vulkan device for the selected GPU
    const vkb::DeviceBuilder device_builder(vkb_gpu);
    const vkb::Device vkb_device = device_builder.build().value();

    device = vkb_device.device;

    // Get the graphics queue attributes
    queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_index = (int)vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void Application::init_swapchain()
{
    // Create a swapchain
    vkb::SwapchainBuilder builder(gpu, device, surface);
    builder.use_default_format_selection();
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR); // use vsync
    builder.set_desired_extent(window_extent.width, window_extent.height);
    vkb::Swapchain vkbSwapchain = builder.build().value();

    swapchain = vkbSwapchain.swapchain;
    image_format = vkbSwapchain.image_format;
    swapchain_images = vkbSwapchain.get_images().value();
    swapchain_image_views = vkbSwapchain.get_image_views().value();
}

void Application::init_default_renderpass()
{
    // Define the attachments to use for the render pass. In this case, we'll
    // use just one color attachment
    const VkAttachmentDescription color_attachment = {
        .format = image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    // Reference to the color attachment for render subpasses
    const VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // Define the attachments for the render subpasses. We'll just use one for
    // now, which will use the color attachment above
    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };

    // Create the render pass
    const VkRenderPassCreateInfo render_pass_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass
    };
    VK_CHECK(vkCreateRenderPass(
        device, 
        &render_pass_create_info, 
        nullptr, 
        &render_pass)
    );
}

void Application::init_framebuffers()
{
    // Create the framebuffers for the swapchain images. This will connect the
    // render-pass to the images for rendering
    VkFramebufferCreateInfo fb_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .width = window_extent.width,
        .height = window_extent.height,
        .layers = 1
    };

    // Allocate memory for framebuffers based on how many images we have
    const int swapchain_image_count = (int)swapchain_images.size();
    framebuffers = std::vector<VkFramebuffer>(swapchain_image_count);

    // Create framebuffers for each swapchain image view
    for (int i = 0; i < swapchain_image_count; i++)
    {
        fb_create_info.pAttachments = &swapchain_image_views[i];
        VK_CHECK(vkCreateFramebuffer(
            device, 
            &fb_create_info, 
            nullptr, 
            &framebuffers[i])
        );
    }
}

VkShaderModule Application::load_shader_module(const char* filename) const
{
    // Open the binary file for reading, with the cursor at the end
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::string err("Failed to open file: ");
        throw std::runtime_error(err + filename);
    }

    // Get the file size using the current position of the cursor
    const size_t file_size = (size_t)file.tellg();

    // Allocate memory for the buffer based on the file size
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    // Move the file cursor to the beginning
    file.seekg(0);

    // Read the file into the buffer
    file.read(reinterpret_cast<char *>(buffer.data()), (int64_t)file_size);

    file.close();

    // Create a new shader module
    const VkShaderModuleCreateInfo shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .codeSize = buffer.size() * sizeof(uint32_t),
        .pCode = buffer.data()
    };

    VkShaderModule shader_module;
    VK_CHECK(vkCreateShaderModule(
        device, 
        &shader_module_create_info,
        nullptr, 
        &shader_module)
    );

    return shader_module;
}

void Application::init_commands()
{
    // Create a Vulkan command pool
    const VkCommandPoolCreateInfo command_pool_create_info =
        vkinit::command_pool_create_info(
            graphics_queue_index,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );
    VK_CHECK(vkCreateCommandPool(
        device, 
        &command_pool_create_info,
        nullptr, 
        &command_pool)
    );

    // Allocate the main command buffer
    const VkCommandBufferAllocateInfo command_buffer_allocate_info =
        vkinit::command_buffer_allocate_info(command_pool, 1);
    VK_CHECK(vkAllocateCommandBuffers(
        device,
        &command_buffer_allocate_info,
        &main_command_buffer)
    );
}

void Application::init_sync_objects()
{
    // Create a Vulkan fence
    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &render_fence));

    // Create semaphores for synchronizing the rendering and presenting pipeline
    // stages
    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
    VK_CHECK(vkCreateSemaphore(
        device, 
        &semaphore_create_info, 
        nullptr, 
        &render_semaphore)
    );
    VK_CHECK(vkCreateSemaphore(
        device,
        &semaphore_create_info,
        nullptr,
        &present_semaphore)
    );
}

void Application::init_pipelines()
{
    // Set up the shaders
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    VkPipelineShaderStageCreateInfo stage;
    VkShaderModule module;

    // Vertex stage
    module = load_shader_module("shaders/spirv/trivert.spv");
    stage = vkinit::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, module);
    shader_stages.push_back(stage);

    // Fragment stage
    module = load_shader_module("shaders/spirv/trifrag.spv");
    stage = vkinit::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, module);
    shader_stages.push_back(stage);

    // Create a pipeline layout
    const VkPipelineLayoutCreateInfo layout_info = vkinit::pipeline_layout_create_info();
    VK_CHECK(vkCreatePipelineLayout(
        device, 
        &layout_info, 
        nullptr, 
        &pipeline_layout)
    );

    // Fill out the pipeline builder
    PipelineBuilder builder = {
        .shader_stages = shader_stages,
        .vertex_input = vkinit::vertex_input_state_create_info(),
        .input_assembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        .viewport = { .x = 0.0f, .y = 0.0f,
                      .width = (float)window_extent.width,
                      .height = (float)window_extent.height,
                      .minDepth = 0.0f, .maxDepth = 1.0f },
        .scissor = { .offset = { 0, 0 }, .extent = window_extent },
        .raster = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL),
        .blend_attachment = vkinit::color_blend_attachment_state(),
        .multisample = vkinit::multisample_state_create_info(),
        .pipeline_layout = pipeline_layout
    };

    // Build the pipeline
    pipeline = builder.build_pipeline(device, render_pass);
}
