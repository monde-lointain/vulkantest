#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

struct SDL_Window;

struct Application
{
	void initialize();
	void run();
	void destroy() const;

	void setup();
	void input();
	void update();
	void render();

	SDL_Window* window = nullptr;
	VkExtent2D window_extent = { 1280, 720 };
	const char* window_name = "Vulkan Renderer";

	int current_frame = 0;
	float target_seconds_per_frame = 0.0f;
	bool running = false;

	void init_vulkan();
	void init_swapchain();
	void init_default_renderpass();
	void init_framebuffers();
	void init_commands();
	void init_sync_objects();
	//void init_pipelines();
	//void init_scene();
	//void init_descriptors();

	/** The Vulkan context. Used to access Vulkan drivers */
	VkInstance instance = nullptr;

	/**
	 * Debug message handler. Passes along debug messages to a designated debug
	 * callback function
	 */
	VkDebugUtilsMessengerEXT debug_messenger = nullptr;

	/**
	 * Represents a GPU installed in the system. What commands are actually
	 * executed on.
	 */
	VkPhysicalDevice gpu = nullptr;

	/**
	 * A logical device created on top of the physical device. Is used for
	 * allocating resources and executing commands on the physical device.
	 */
	VkDevice device = nullptr;

	/**
	 * Represents a platform-specific surface/window that is used for
	 * presenting rendered graphics to the user
	 */
	VkSurfaceKHR surface = nullptr;

	/** Collection of images that are rendered to a Vulkan surface */
	VkSwapchainKHR swapchain = nullptr;

	VkFormat image_format = VK_FORMAT_UNDEFINED;

	/**
	 * In Vulkan images are not directly accesible by pipeline shaders for
	 * reading or writing to. Instead, images must be accessed through image
	 * view objects. An image view object contains data on how to access the
	 * image object's data, specifying things such as the format and dimensions,
	 * as well as how to handle multi-planar images, cube maps, or array
	 * texture.
	 */
	std::vector<VkImage> swapchain_images = std::vector<VkImage>();
	std::vector<VkImageView> swapchain_image_views = std::vector<VkImageView>();

	/**
	 * Defines the collection of attachments and subpasses that define the
	 * sequence of rendering operations
	 */
	VkRenderPass render_pass = nullptr;

	/** The main buffer for writing commands into */
	VkCommandBuffer main_command_buffer;

	/** Manages memory for the command buffer */
	VkCommandPool command_pool;

	/**
	 * Array of framebuffers to be written into by a Vulkan render pass. A
	 * Vulkan framebuffer is a handle to a collection of attachments that define
	 * a rendering target for a render pass object
	 */
	std::vector<VkFramebuffer> framebuffers;

	/** Attributes for the graphics queue family*/
	VkQueue graphics_queue = nullptr;
	uint32_t graphics_queue_family_index = 0;

	/**
	 * Synchronizes access to the graphics queue for rendering images and
	 * presenting them to the screen
	 */
	VkSemaphore render_semaphore = nullptr;
	VkSemaphore present_semaphore = nullptr;

	/**
	 * Synchronizes the CPU and the GPU to wait until the GPU has finished
	 * rendering
	 */
	VkFence render_fence = nullptr;
};