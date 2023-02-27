#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan_core.h>

#include "Model/Model.h"

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 modelviewprojection;
};

enum ERenderMode
{
    RAINBOW,
    SOLID,
};

struct PipelineBuilder;
struct SDL_Window;

struct Application
{
    void initialize();

    void run();

    void destroy();

    void setup();

    void input();

    void update();

    void render();

    SDL_Window* window = nullptr;
    VkExtent2D window_extent = { 800, 600 };
    VkRect2D scissor = { { 0, 0 }, window_extent };
    const char* window_name = "Vulkan Renderer";

    int current_frame = 0;
    float target_seconds_per_frame = 0.0f;
    bool running = false;

    ERenderMode render_mode = SOLID;

    void init_instance();

    void init_allocator();

    void init_swapchain();

    void init_default_renderpass();

    void init_framebuffers();

    VkShaderModule load_shader_module(const char* filename) const;

    void init_commands();

    void init_sync_objects();

    void init_pipelines();

    void pipe_cleanup(
        PipelineBuilder& builder, 
        std::vector<VkPipelineShaderStageCreateInfo>& shader_stages
    ) const;

    //void init_scene();

    //void init_descriptors();

    void destroy_vulkan_resources();

    /** The VMA allocator. Manages memory buffers */
    VmaAllocator allocator = nullptr;

    /** The Vulkan instance. Used to access Vulkan drivers. */
    VkInstance instance = nullptr;

    /**
     * The Vulkan debug message handler. Passes along debug messages to a
     * designated debug callback function.
     */
    VkDebugUtilsMessengerEXT debug_messenger = nullptr;

    /** The Vulkan physical device. Represents an actual GPU on the system. */
    VkPhysicalDevice gpu = nullptr;

    /**
     * The Vulkan device. Used for allocating resources and executing commands
     * on the physical device.
     */
    VkDevice device = nullptr;

    /** The Vulkan device queue. */
    VkQueue queue = nullptr;

    /**
     * The Vulkan surface. Represents a platform-specific surface/window used
     * for displaying rendered graphics.
     */
    VkSurfaceKHR surface = nullptr;

    /** A collection of images that are rendered to a Vulkan surface. */
    VkSwapchainKHR swapchain = nullptr;

    /** Pixel format of the swapchain. */
    VkFormat image_format = VK_FORMAT_UNDEFINED;

    /** Index to the queue family graphics commands are submitted to. */
    int graphics_queue_index = -1;

    /**
     * Arrays for swap chain images and swap chain image views. In Vulkan,
     * images are not directly accesible by pipeline shaders for reading or
     * writing to and must be accessed through image view objects instead. An
     * image view object contains data on how to access the image's data,
     * specifying things like the image format and dimensions, as well as how to
     * handle multi-planar images, cube maps, or array texture.
     */
    std::vector<VkImageView> swapchain_image_views = std::vector<VkImageView>();
    std::vector<VkImage> swapchain_images = std::vector<VkImage>();

    /** Image buffer to store depth values in */
    VkImageView depth_image_view = {};
    Image depth_image = {};

    /** Format of the depth image */
    VkFormat depth_format = VK_FORMAT_UNDEFINED;

    /**
     * Array of framebuffers for each swapchain image view. A Vulkan framebuffer
     * is a handle to a collection of attachments that define a rendering target
     * for a render pass object.
     */
    std::vector<VkFramebuffer> framebuffers;

    /**
     * The render pass description. Defines the collection of attachments and
     * subpasses that make up a render pass.
     */
    VkRenderPass render_pass = nullptr;

    /** Graphics pipeline for triangle filling */
    VkPipeline pipeline = nullptr;

    /** Pipeline layouts for resources. */
    VkPipelineLayout pipeline_layout = nullptr;

    /** Contains render commands for vertex targets. */
    VkCommandBuffer main_command_buffer = nullptr;

    /** Manages memory for the command buffer. */
    VkCommandPool command_pool = nullptr;

    /**
     * Synchronizes access to the graphics queue for rendering images and
     * presenting them to the screen.
     */
    VkSemaphore render_semaphore = nullptr;
    VkSemaphore present_semaphore = nullptr;

    /**
     * Synchronizes the CPU and the GPU to wait until the GPU has finished
     * rendering.
     */
    VkFence render_fence = nullptr;

    void load_models();

    void upload_model(std::unique_ptr<Model>& model);

    Model triangle = {};
    std::unique_ptr<Model> cube = nullptr;

    std::vector<std::unique_ptr<Model>> models;
};