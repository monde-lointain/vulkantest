#pragma once

#include <array>
#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan_core.h>

#include "Camera/Camera.h"
#include "Model/Model.h"
#include "Scene/Scene.h"
#include "VulkanRenderer/DeletionQueue.h"
#include "Window/Window.h"

const int NUM_OVERLAPPING_FRAMES = 3;
const int MAX_DESCRIPTOR_SETS = 10;
const int MAX_OBJECTS = 10000;

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 modelviewprojection;
};

struct GPUObjectData
{
    glm::mat4 model_matrix;
};

struct UploadContext {
    VkFence upload_fence;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
};


/** Per-frame data */
struct PerFrame
{
    VkFence queue_submit_fence = nullptr;

    VkCommandPool primary_command_pool = nullptr;

    VkCommandBuffer primary_command_buffer = nullptr;

    VkSemaphore swapchain_acquire_semaphore = nullptr;

    VkSemaphore swapchain_release_semaphore = nullptr;

    Buffer global_uniform_buffer = {};

    VkDescriptorSet global_descriptor_set = nullptr;

    Buffer object_storage_buffer = {};

    VkDescriptorSet object_descriptor_set = nullptr;
};

/** Vulkan objects and global state */
struct Context
{
    /** The VMA allocator. Manages memory buffers. */
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
    VkPhysicalDeviceProperties gpu_properties;

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
     * Array for swap chain image views. In Vulkan, images are not directly
     * accesible by pipeline shaders for reading or writing to and must be
     * accessed through image view objects instead. An image view object
     * contains data on how to access the image's data, specifying things like
     * the image format and dimensions, as well as how to handle multi-planar
     * images, cube maps, or array texture.
     */
    std::vector<VkImageView> swapchain_image_views;

    /** Image buffer to store depth values in. */
    Image depth_image;
    VkImageView depth_image_view;

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

    /** Graphics pipeline for triangle filling. */
    VkPipeline pipeline = nullptr;

    /** Pipeline layouts for resources. */
    VkPipelineLayout pipeline_layout = nullptr;

    /** Per-frame data. */
    std::array<PerFrame, NUM_OVERLAPPING_FRAMES> frames;

    /** Describes layouts of the descriptor sets. */
    VkDescriptorSetLayout global_descriptor_set_layout = nullptr;
    VkDescriptorSetLayout object_descriptor_set_layout = nullptr;

    /**
     * A pool of descriptor sets, which are allocated by the application at
     * runtime.
     */
    VkDescriptorPool descriptor_pool = nullptr;

    /**
     * Scene data and buffer. Contains parameters for the scene environent such
     * as fog and sunlight
     */
    Scene scene_data;
    Buffer scene_data_buffer;
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

    // SDL_Window* window = nullptr;
    // VkExtent2D window_extent = { 800, 600 };
    // VkRect2D scissor = { { 0, 0 }, window_extent };
    // const char* window_name = "Vulkan Renderer";

    std::unique_ptr<Window> window = std::make_unique<Window>();

    int current_frame = 0;
    bool running = false;

    ERenderMode render_mode = SOLID;

    void init_instance();

    void init_allocator();

    void init_swapchain();

    void init_default_renderpass();

    void init_framebuffers();

    VkShaderModule load_shader_module(const char *filename) const;

    void init_per_frames();

    void init_pipelines();

    void pipe_cleanup(
        PipelineBuilder &builder,
        std::vector<VkPipelineShaderStageCreateInfo> &shader_stages
    ) const;

    // void init_scene();

    void init_descriptors();

    void destroy_vulkan_resources();

    //void destroy_per_frames();

    Context context;

    UploadContext upload_context;

    DeletionQueue deletion_queue;

    void load_models();

    [[nodiscard]]
    Buffer create_buffer(
        size_t alloc_size,
        VkBufferUsageFlags usage, 
        VmaMemoryUsage memory_usage
    ) const;

    void upload_model(std::unique_ptr<Model> &model);

    PerFrame &get_current_frame();

    Model triangle = {};

    std::vector<std::unique_ptr<Model>> models;

    Camera camera;

    size_t pad_uniform_buffer_size(size_t size);

    void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);
};