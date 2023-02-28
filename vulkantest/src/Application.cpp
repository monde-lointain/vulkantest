#include "Application.h"

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk-bootstrap/VkBootstrap.h>

#include "Utils/string_ops.h"
#include "VulkanRenderer/PipelineBuilder.h"
#include "VulkanRenderer/vkinit.h"
#include "VulkanRenderer/vkutils.h"

#pragma warning(push, 0)
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#pragma warning(pop)

#ifdef NDEBUG
const bool VALIDATION_LAYERS_ON = false;
#else
const bool VALIDATION_LAYERS_ON = true;
#endif

void Application::setup()
{
    // Load models into the scene
    load_models();

    // Initialize the camera
    camera.position = glm::vec3(0.0f, 8.0f, 30.0f);
    camera.aspect = (float)window->extent.width / (float)window->extent.height;
    camera.set_projection();
}

void Application::input()
{
    SDL_Event event;

    // Handling core SDL events (moving the mouse, closing the window, etc.)
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // Closing the window
            case SDL_QUIT:
            {
                running = false;
                break;
            }
            // Clicking the window
            case SDL_MOUSEBUTTONDOWN:
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    window->clicked();
                    camera.input_mode = INPUT_ENABLED;
                    camera.window_clicked = true;
                    break;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    window->released();
                    camera.input_mode = MOUSE_INPUT_DISABLED;
                    camera.window_clicked = false;
                    camera.mouse_has_position = false;
                    break;
                }
                break;
            }
            //  Adjusting the camera speed with the mouse
            case SDL_MOUSEWHEEL:
            {
                if (event.wheel.y > 0)
                {
                    camera.speed += camera.SPEED_INC;
                    camera.speed = glm::clamp(
                        camera.speed,
                        camera.MIN_SPEED,
                        camera.MAX_SPEED
                    );
                }
                else if (event.wheel.y < 0)
                {
                    camera.speed -= camera.SPEED_INC;
                    camera.speed = glm::clamp(
                        camera.speed,
                        camera.MIN_SPEED,
                        camera.MAX_SPEED
                    );
                }
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                    break;
                }
                /**
                 * Camera controls
                 * Note: The camera controls are handled by ORing together
                 * different flags on the move state bitmask, which allows us to
                 * easily handle multiple keypresses at once by simply setting
                 * and removing bits from the mask
                 */
                if (event.key.keysym.sym == SDLK_w)
                {
                    camera.set_move_state(FORWARD, true);
                    break;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    camera.set_move_state(BACKWARD, true);
                    break;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    camera.set_move_state(RIGHT, true);
                    break;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    camera.set_move_state(LEFT, true);
                    break;
                }
                if (event.key.keysym.sym == SDLK_q)
                {
                    camera.set_move_state(DOWN, true);
                    break;
                }
                if (event.key.keysym.sym == SDLK_e)
                {
                    camera.set_move_state(UP, true);
                    break;
                }
                break;
            }
            case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_w)
                {
                    camera.set_move_state(FORWARD, false);
                    break;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    camera.set_move_state(BACKWARD, false);
                    break;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    camera.set_move_state(RIGHT, false);
                    break;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    camera.set_move_state(LEFT, false);
                    break;
                }
                if (event.key.keysym.sym == SDLK_q)
                {
                    camera.set_move_state(DOWN, false);
                    break;
                }
                if (event.key.keysym.sym == SDLK_e)
                {
                    camera.set_move_state(UP, false);
                    break;
                }
            }
        }
    }
}

void Application::update()
{
    camera.update();

    for (const std::unique_ptr<Model>& model : models)
    {
        float rot = (float)current_frame * 0.005f;
        model->rotation.y = rot;
        model->update();
    }
}

constexpr uint64_t ONE_SECOND = 1000000000;
constexpr uint64_t TIMEOUT_PERIOD = ONE_SECOND;

void Application::render()
{
    // Get current frame data
    const PerFrame& frame = get_current_frame();

    // Wait until the GPU has finished rendering, looping in case it takes
    // longer than expected
    VkResult result;
    do
    {
        result = vkWaitForFences(
            context.device, 
            1, 
            &frame.queue_submit_fence,
            true, 
            TIMEOUT_PERIOD
        );
    } while (result == VK_TIMEOUT);
    VK_CHECK(vkResetFences(context.device, 1, &frame.queue_submit_fence));

    // Request an image from swapchain
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(
        context.device,
        context.swapchain,
        TIMEOUT_PERIOD,
        frame.swapchain_acquire_semaphore,
        nullptr, 
        &swapchain_image_index)
    );

    // Clear all command buffers
    VK_CHECK(vkResetCommandPool(context.device, frame.primary_command_pool, 0));

    // Start recording commands into the command buffer
    const VkCommandBufferBeginInfo cmd_buf_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };
    VK_CHECK(vkBeginCommandBuffer(
        frame.primary_command_buffer,
        &cmd_buf_begin_info)
    );

    // Set color clear value
    VkClearValue color_clear_value;
    //float flash = fabsf(sinf((float)current_frame / 120.f));
    color_clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    // Set depth clear value
    VkClearValue depth_clear_value;
    depth_clear_value.depthStencil.depth = 1.0f;

    const std::array<VkClearValue, 2> clear_values = {
        color_clear_value, 
        depth_clear_value
    };

    // Begin a render pass
    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = context.render_pass,
        .framebuffer = context.framebuffers[swapchain_image_index],
        .renderArea = window->scissor,
        .clearValueCount = (uint32_t)clear_values.size(),
        .pClearValues = &clear_values.data()[0]
    };
    vkCmdBeginRenderPass(
        frame.primary_command_buffer,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // Bind pipeline to the command buffer
    vkCmdBindPipeline(
        frame.primary_command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        context.pipeline
    );

    // Bind descriptor sets to pipeline
    vkCmdBindDescriptorSets(
        frame.primary_command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        context.pipeline_layout,
        0, 
        1, 
        &frame.mvp_descriptor_set,
        0, 
        nullptr
    );

    // Loop over all models in the scene
    for (const std::unique_ptr<Model> &model : models)
    {
        // Bind vertex buffer to the command buffer with an offset of zero
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(frame.primary_command_buffer, 0, 1,
            &model->vertex_buffer.buffer, &offset);

        // Create MVP matrix
        const glm::mat4 modelviewprojection =
            camera.vp_matrix * model->transform;

        // Copy MVP into the uniform buffer
        void *data;
        vmaMapMemory(
            context.allocator, frame.mvp_uniform_buffer.allocation, &data);

        memcpy(data, &modelviewprojection, sizeof(glm::mat4));

        vmaUnmapMemory(context.allocator, frame.mvp_uniform_buffer.allocation);

        // Draw the model
        vkCmdDraw(frame.primary_command_buffer,
            (uint32_t)model->vertices.size(), 1, 0, 0);
    }

    // Finalize render stage commands
    vkCmdEndRenderPass(frame.primary_command_buffer);
    VK_CHECK(vkEndCommandBuffer(frame.primary_command_buffer));

    // Submit command buffer to the graphics queue
    const VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.swapchain_acquire_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame.primary_command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frame.swapchain_release_semaphore
    };
    VK_CHECK(vkQueueSubmit(
        context.queue, 
        1, 
        &submit_info, 
        frame.queue_submit_fence)
    );

    // Present image to the swap chain
    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.swapchain_release_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &context.swapchain,
        .pImageIndices = &swapchain_image_index
    };
    VK_CHECK(vkQueuePresentKHR(context.queue, &present_info));
}

void Application::initialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, window->name,
            "Failed to initialize SDL.", nullptr);
        return;
    }

    // Create a window
    window->init();
    if (!window->window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, window->name,
            "Failed to initialize SDL window.", nullptr);
        SDL_Quit();
        return;
    }

    // Initialize the Vulkan renderer
    init_instance();
    init_allocator();
    init_swapchain();
    init_default_renderpass();
    init_framebuffers();
    init_per_frames();
    init_descriptors();
    //init_sync_objects();
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

void Application::destroy()
{
    destroy_vulkan_resources();

    // Free SDL resources
    window->destroy();
}

void Application::destroy_per_frames()
{
    for (const PerFrame &frame : context.frames)
    {
        if (frame.swapchain_acquire_semaphore)
        {
            vkDestroySemaphore(
                context.device, frame.swapchain_acquire_semaphore, nullptr);
        }
        if (frame.swapchain_release_semaphore)
        {
            vkDestroySemaphore(
                context.device, frame.swapchain_release_semaphore, nullptr);
        }
        if (frame.queue_submit_fence)
        {
            vkDestroyFence(context.device, frame.queue_submit_fence, nullptr);
        }
        if (frame.primary_command_pool)
        {
            vkDestroyCommandPool(
                context.device, frame.primary_command_pool, nullptr);
        }
        if (frame.mvp_uniform_buffer.buffer)
        {
            vmaDestroyBuffer(context.allocator, frame.mvp_uniform_buffer.buffer,
                frame.mvp_uniform_buffer.allocation);
        }
    }
}

void Application::destroy_vulkan_resources()
{
    // Wait until the GPU is completely idle
    vkDeviceWaitIdle(context.device);

    for (const std::unique_ptr<Model> &model : models)
    {
        if (model->vertex_buffer.buffer)
        {
            vmaDestroyBuffer(context.allocator, model->vertex_buffer.buffer,
                model->vertex_buffer.allocation);
        }
    }
    if (context.pipeline)
    {
        vkDestroyPipeline(context.device, context.pipeline, nullptr);
    }
    if (context.pipeline_layout)
    {
        vkDestroyPipelineLayout(
            context.device, context.pipeline_layout, nullptr);
    }
    if (context.descriptor_set_layout)
    {
        vkDestroyDescriptorSetLayout(
            context.device, context.descriptor_set_layout, nullptr);
    }
    if (context.descriptor_pool)
    {
        vkDestroyDescriptorPool(
            context.device, context.descriptor_pool, nullptr);
    }
    destroy_per_frames();
    if (context.render_pass)
    {
        vkDestroyRenderPass(context.device, context.render_pass, nullptr);
    }
    if (context.depth_image_view)
    {
        vkDestroyImageView(context.device, context.depth_image_view, nullptr);
    }
    if (context.depth_image.allocation)
    {
        vmaDestroyImage(context.allocator, context.depth_image.image,
            context.depth_image.allocation);
    }
    if (context.swapchain)
    {
        vkDestroySwapchainKHR(context.device, context.swapchain, nullptr);
    }
    for (size_t i = 0; i < context.framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(context.device, context.framebuffers[i], nullptr);
        vkDestroyImageView(
            context.device, context.swapchain_image_views[i], nullptr);
    }
    if (context.allocator)
    {
        vmaDestroyAllocator(context.allocator);
    }
    if (context.surface)
    {
        vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
    }
    if (context.device)
    {
        vkDestroyDevice(context.device, nullptr);
    }
    vkb::destroy_debug_utils_messenger(
        context.instance, context.debug_messenger);
    vkDestroyInstance(context.instance, nullptr);
}

void Application::init_instance()
{
    // Create a Vulkan instance and debug messenger
    vkb::InstanceBuilder builder;
    builder.set_app_name("Vulkan Renderer");
    builder.require_api_version(1, 3, 0);
    if (VALIDATION_LAYERS_ON)
    {
        builder.request_validation_layers(true); // Enables "VK_LAYER_KHRONOS_validation"
        /*builder.enable_layer("VK_LAYER_LUNARG_api_dump");*/
        builder.use_default_debug_messenger();
    }
    const vkb::Instance vkb_inst = builder.build().value();

    context.instance = vkb_inst.instance;
    context.debug_messenger = vkb_inst.debug_messenger;

    // Create a Vulkan surface to draw to
    SDL_Vulkan_CreateSurface(window->window, context.instance, &context.surface);

    // Select a GPU from the available physical devices
    vkb::PhysicalDeviceSelector selector(vkb_inst);
    selector.set_minimum_version(1, 3);
    selector.set_surface(context.surface);
    const vkb::PhysicalDevice vkb_gpu = selector.select().value();

    context.gpu = vkb_gpu.physical_device;

    // Create a Vulkan device for the selected GPU
    const vkb::DeviceBuilder device_builder(vkb_gpu);
    const vkb::Device vkb_device = device_builder.build().value();

    context.device = vkb_device.device;

    // Get graphics queue attributes
    context.queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    context.graphics_queue_index =
        (int)vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void Application::init_allocator()
{
    const VmaAllocatorCreateInfo allocator_create_info = {
        .physicalDevice = context.gpu,
        .device = context.device,
        .instance = context.instance
    };
    VK_CHECK(vmaCreateAllocator(&allocator_create_info, &context.allocator));
}

void Application::init_swapchain()
{
    // Create a swapchain
    vkb::SwapchainBuilder builder(context.gpu, context.device, context.surface);
    builder.use_default_format_selection();
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR); // use vsync
    builder.set_desired_min_image_count(vkb::SwapchainBuilder::TRIPLE_BUFFERING);
    builder.set_desired_extent(window->extent.width, window->extent.height);
    vkb::Swapchain vkbSwapchain = builder.build().value();

    context.swapchain = vkbSwapchain.swapchain;
    context.image_format = vkbSwapchain.image_format;
    context.swapchain_image_views = vkbSwapchain.get_image_views().value();

    // Configure the depth image
    const VkExtent3D depth_image_extent = {
        .width = window->extent.width,
        .height = window->extent.height,
        .depth = 1
    };

    context.depth_format = VK_FORMAT_D32_SFLOAT;

    const VkImageCreateInfo image_create_info = vkinit::image_create_info(
        context.depth_format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        depth_image_extent
    );

    // Create depth image
    const VmaAllocationCreateInfo image_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    VK_CHECK(vmaCreateImage(
        context.allocator,
        &image_create_info,
        &image_alloc_info,
        &context.depth_image.image,
        &context.depth_image.allocation,
        nullptr)
    );

    // Create depth image view
    const VkImageViewCreateInfo image_view_create_info =
        vkinit::imageview_create_info(
            context.depth_format,
            context.depth_image.image,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );

    VK_CHECK(vkCreateImageView(
        context.device,
        &image_view_create_info, 
        nullptr, 
        &context.depth_image_view)
    );
}

void Application::init_default_renderpass()
{
    // Define attachments to use for the render pass
    // Color attachment
    const VkAttachmentDescription color_attachment = {
        .flags = 0,
        .format = context.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    const VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // Depth attachment
    const VkAttachmentDescription depth_attachment =
    {
        .flags = 0,
        .format = context.depth_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    const VkAttachmentReference depth_attachment_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // Define attachments for render subpass
    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref
    };

    // Define attachments for render pass
    std::array<VkAttachmentDescription, 2> attachments = {
        color_attachment,
        depth_attachment
    };

    // Create dependencies for color and depth attachments
    const VkSubpassDependency color_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    // Sycnhronize access to depth attachments
    const VkSubpassDependency depth_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    };

    // Define dependencies for render pass
    std::array<VkSubpassDependency, 2> dependencies = {
        color_dependency, 
        depth_dependency
    };

    // Create render pass
    const VkRenderPassCreateInfo render_pass_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = (uint32_t)attachments.size(),
        .pAttachments = &attachments.data()[0],
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = (uint32_t)dependencies.size(),
        .pDependencies = &dependencies.data()[0]
    };
    VK_CHECK(vkCreateRenderPass(
        context.device,
        &render_pass_create_info, 
        nullptr, 
        &context.render_pass)
    );
}

void Application::init_framebuffers()
{
    // Create framebuffers for the swapchain images. This will connect the
    // render pass to the images for rendering
    VkFramebufferCreateInfo fb_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = context.render_pass,
        .attachmentCount = 1,
        .width = window->extent.width,
        .height = window->extent.height,
        .layers = 1
    };

    // Allocate memory for framebuffers based on how many images we have
    const int image_view_count = (int)context.swapchain_image_views.size();
    context.framebuffers = std::vector<VkFramebuffer>(image_view_count);

    std::array<VkImageView, 2> attachments = {};
    // Create framebuffers for each swapchain image view
    for (int i = 0; i < image_view_count; i++)
    {
        attachments[0] = context.swapchain_image_views[i];
        attachments[1] = context.depth_image_view;

        fb_create_info.attachmentCount = (uint32_t)attachments.size();
        fb_create_info.pAttachments = attachments.data();

        VK_CHECK(vkCreateFramebuffer(
            context.device,
            &fb_create_info, 
            nullptr, 
            &context.framebuffers[i])
        );
    }
}

VkShaderModule Application::load_shader_module(const char* filename) const
{
    // Open binary file for reading and seek to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        const std::string err("Failed to open file: ");
        throw std::runtime_error(err + filename);
    }

    // Use position of the cursor to determine file size
    const size_t file_size = (size_t)file.tellg();

    // Allocate memory for buffer based on the file size
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    // Read file into the buffer
    file.seekg(0);
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
        context.device,
        &shader_module_create_info,
        nullptr, 
        &shader_module)
    );

    return shader_module;
}

void Application::init_per_frames()
{
    const VkCommandPoolCreateInfo command_pool_create_info =
        vkinit::command_pool_create_info(
            context.graphics_queue_index, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };

    // For each swapchain image
    for (PerFrame& frame : context.frames)
    {
        // Create a Vulkan command pool
        VK_CHECK(vkCreateCommandPool(context.device, &command_pool_create_info,
            nullptr, &frame.primary_command_pool));

        // Allocate a command buffer from the pool
        const VkCommandBufferAllocateInfo command_buffer_allocate_info =
            vkinit::command_buffer_allocate_info(frame.primary_command_pool, 1);

        VK_CHECK(vkAllocateCommandBuffers(context.device,
            &command_buffer_allocate_info, &frame.primary_command_buffer));

        // Create a Vulkan fence
        VK_CHECK(vkCreateFence(context.device, &fence_create_info, nullptr,
            &frame.queue_submit_fence));

        // Create semaphores to synchornize acquiring images from the swapchain
        VK_CHECK(vkCreateSemaphore(context.device, &semaphore_create_info,
            nullptr, &frame.swapchain_acquire_semaphore));
        VK_CHECK(vkCreateSemaphore(context.device, &semaphore_create_info,
            nullptr, &frame.swapchain_release_semaphore));
    }
}

void Application::init_descriptors()
{

    // Create a descriptor set layout for the UBO
    const VkDescriptorSetLayoutBinding layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    const VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &layout_binding
    };

    VK_CHECK(vkCreateDescriptorSetLayout(
        context.device, 
        &layout_info, 
        nullptr, 
        &context.descriptor_set_layout)
    );

    // Create a descriptor pool to allocate the descriptor sets
    const VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MAX_DESCRIPTOR_SETS
    };

    const VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = MAX_DESCRIPTOR_SETS,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };

    VK_CHECK(vkCreateDescriptorPool(
        context.device, 
        &descriptor_pool_create_info, 
        nullptr, 
        &context.descriptor_pool)
    );

    VkDescriptorSetAllocateInfo alloc_info;
    VkDescriptorBufferInfo buffer_info;
    VkWriteDescriptorSet descriptor_write;
    for (PerFrame& frame : context.frames)
    {
        // Create a uniform buffer object
        frame.mvp_uniform_buffer = create_buffer(sizeof(glm::mat4),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        // Allocate a descriptor set
        alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = context.descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &context.descriptor_set_layout
        };
        VK_CHECK(vkAllocateDescriptorSets(
            context.device, 
            &alloc_info, 
            &frame.mvp_descriptor_set)
        );

        // Bind uniform buffer object to the descriptor set
        buffer_info = {
            .buffer = frame.mvp_uniform_buffer.buffer,
            .offset = 0,
            .range = sizeof(glm::mat4)
        };

        descriptor_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = frame.mvp_descriptor_set,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info
        };

        vkUpdateDescriptorSets(
            context.device, 1, &descriptor_write, 0, nullptr);
    }
}

void Application::init_pipelines()
{
    // Set up shaders for pipeline
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    VkShaderModule module;
    VkPipelineShaderStageCreateInfo stage;

    // Load shaders for pipeline
    // Vertex stage
    module = load_shader_module("shaders/spirv/tri_mesh.spv");
    stage = vkinit::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, module);
    shader_stages.push_back(stage);

    // Fragment stage
    module = load_shader_module("shaders/spirv/tri_frag.spv");
    stage = vkinit::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, module);
    shader_stages.push_back(stage);

    // Specify vertex input descriptors for pipeline
    VertexInputDescription description = get_vertex_input_description();

    // Fill pipeline layout struct
    VkPipelineLayoutCreateInfo layout_info = vkinit::pipeline_layout_create_info();

    // Specify descriptor set layout
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &context.descriptor_set_layout;

    // Create pipeline layout
    VK_CHECK(vkCreatePipelineLayout(
        context.device, 
        &layout_info, 
        nullptr, 
        &context.pipeline_layout)
    );

    // Fill out the pipeline builder
    PipelineBuilder builder = {
        .shader_stages = shader_stages,
        .vertex_input = vkinit::vertex_input_state_create_info(description),
        .input_assembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        .viewport = { .x = 0.0f, .y = 0.0f,
                      .width = (float)window->extent.width,
                      .height = (float)window->extent.height,
                      .minDepth = 0.0f, .maxDepth = 1.0f },
        .scissor = window->scissor,
        .raster = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL),
        .blend_attachment = vkinit::color_blend_attachment_state(),
        .multisample = vkinit::multisample_state_create_info(),
        .depth_stencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
        .pipeline_layout = context.pipeline_layout
    };

    // Build the pipeline
    builder.shader_stages = shader_stages;
    context.pipeline = builder.build_pipeline(context.device, context.render_pass);

    // Cleanup pipeline resources
    pipe_cleanup(builder, shader_stages);
}

void Application::load_models()
{
    // Allocate memory for the triangle vertices
    triangle.vertices.resize(3);

    // Define vertex attributes for the triangle
    triangle.vertices[0].position = { -0.5f,  0.5f, 0.0f }; // bottom left
    triangle.vertices[1].position = {  0.5f,  0.5f, 0.0f }; // bottom right
    triangle.vertices[2].position = {  0.0f, -0.5f, 0.0f }; // top
    triangle.vertices[0].color = { 1.0f, 0.0f, 0.0f };
    triangle.vertices[1].color = { 0.0f, 1.0f, 0.0f };
    triangle.vertices[2].color = { 0.0f, 0.0f, 1.0f };

    std::unique_ptr<Model> cube = create_model("assets/models/koopa/koopa.obj");

    if (cube)
    {
        upload_model(cube);
    }
}

Buffer Application::create_buffer(
    size_t alloc_size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memory_usage
) const
{
    // Specify info about the buffer to create
    const VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = alloc_size,
        .usage = usage
    };

    // Set allocation properties for the buffer
    const VmaAllocationCreateInfo alloc_info = { .usage = memory_usage };

    Buffer buffer;

    // Allocate memory for the buffer
    VK_CHECK(vmaCreateBuffer(
        context.allocator, 
        &buffer_info, 
        &alloc_info,
        &buffer.buffer, 
        &buffer.allocation, 
        nullptr)
    );

    return buffer;
}


void Application::upload_model(std::unique_ptr<Model>& model)
{
    // Specify info about the vertex buffer to create
    const VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = model->vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    // Set allocation properties for the buffer
    const VmaAllocationCreateInfo alloc_create_info = {
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    // Allocate memory for the vertex buffer
    VK_CHECK(vmaCreateBuffer(
        context.allocator,
        &buffer_create_info,
        &alloc_create_info,
        &model->vertex_buffer.buffer,
        &model->vertex_buffer.allocation,
        nullptr)
    );

    // Map allocated memory to be accessible to the CPU
    void* vertex_data;
    vmaMapMemory(context.allocator, model->vertex_buffer.allocation, &vertex_data);

    // Copy the model vertex data to the allocated memory block
    const size_t buf_sz = model->vertices.size() * sizeof(Vertex);
    memcpy(vertex_data, model->vertices.data(), buf_sz);

    // Unmap the memory to release it back to the allocator
    vmaUnmapMemory(context.allocator, model->vertex_buffer.allocation);

    // Add model to models in scene
    models.push_back(std::move(model));
}

PerFrame& Application::get_current_frame()
{
    return context.frames[current_frame % NUM_OVERLAPPING_FRAMES];
}

void Application::pipe_cleanup(
    PipelineBuilder &builder,
    std::vector<VkPipelineShaderStageCreateInfo> &shader_stages
) const
{
    // Destroy shader modules
    vkDestroyShaderModule(
        context.device, 
        builder.shader_stages[0].module, 
        nullptr
    );
    vkDestroyShaderModule(
        context.device, 
        builder.shader_stages[1].module, 
        nullptr
    );

    // Clear arrays
    builder.shader_stages.clear();
    shader_stages.clear();
}
