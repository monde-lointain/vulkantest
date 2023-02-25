#include "Application.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk-bootstrap/VkBootstrap.h>

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

	result = vkResetFences(device, 1, &render_fence);
	check_vk_result(result);

	// Request an image from swapchain
	uint32_t swapchain_image_index;
	result = vkAcquireNextImageKHR(
		device, 
		swapchain, 
		TIMEOUT_PERIOD,
		present_semaphore, 
		nullptr, 
		&swapchain_image_index
	);
	check_vk_result(result);

	// Clear the current command buffer
	result = vkResetCommandBuffer(main_command_buffer, 0);
	check_vk_result(result);

	// Begin recording commands into the command buffer
	VkCommandBufferBeginInfo cmd_buf_begin_info = {};
	cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_begin_info.pNext = nullptr;
	cmd_buf_begin_info.pInheritanceInfo = nullptr;
	cmd_buf_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer(main_command_buffer, &cmd_buf_begin_info);
	check_vk_result(result);

	// Set the draw color for clearing the screen
	VkClearValue clear_value;
	float flash = fabsf(sinf((float)current_frame / 120.f));
	clear_value.color = { { flash, flash, flash, 1.0f } };

	// Create a render pass
	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = render_pass;
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = window_extent;
	render_pass_begin_info.framebuffer = framebuffers[swapchain_image_index];
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_value;

	vkCmdBeginRenderPass(
		main_command_buffer, 
		&render_pass_begin_info,
		VK_SUBPASS_CONTENTS_INLINE
	);

	// Finalize the render stage commands
	vkCmdEndRenderPass(main_command_buffer);
	result = vkEndCommandBuffer(main_command_buffer);
	check_vk_result(result);

	// Submit the command buffer to the graphics queue
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;

	const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &present_semaphore;

	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &render_semaphore;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &main_command_buffer;

	result = vkQueueSubmit(queue, 1, &submit_info, render_fence);
	check_vk_result(result);

	// Present the image to the swap chain
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = nullptr;

	present_info.pSwapchains = &swapchain;
	present_info.swapchainCount = 1;

	present_info.pWaitSemaphores = &render_semaphore;
	present_info.waitSemaphoreCount = 1;

	present_info.pImageIndices = &swapchain_image_index;

	result = vkQueuePresentKHR(queue, &present_info);
	check_vk_result(result);
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

	window = SDL_CreateWindow(
		window_name,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(int)window_extent.width, (int)window_extent.height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS
	);

	// Create a window
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
	init_vulkan();
	init_swapchain();
	init_default_renderpass();
	init_framebuffers();
	init_commands();
	//init_descriptors();
	init_sync_objects();
	//init_pipelines();

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
	for (int i = 0; i < framebuffers.size(); i++)
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

void Application::init_vulkan()
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
	graphics_queue_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
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
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Reference to the color attachment for render subpasses
	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Define the attachments for the render subpasses. We'll just use one for
	// now, which will use the color attachment above
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	// Create the render pass
	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;

	const VkResult result = vkCreateRenderPass(
		device, 
		&render_pass_create_info, 
		nullptr, 
		&render_pass
	);
	check_vk_result(result);
}

void Application::init_framebuffers()
{
	// Create the framebuffers for the swapchain images. This will connect the
	// render-pass to the images for rendering
	VkFramebufferCreateInfo fb_create_info = {};
	fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_create_info.pNext = nullptr;
	fb_create_info.renderPass = render_pass;
	fb_create_info.attachmentCount = 1;
	fb_create_info.width = window_extent.width;
	fb_create_info.height = window_extent.height;
	fb_create_info.layers = 1;

	// Allocate memory for framebuffers based on how many images we have
	const int swapchain_image_count = (int)swapchain_images.size();
	framebuffers = std::vector<VkFramebuffer>(swapchain_image_count);

	VkResult result;
	// Create framebuffers for each of the image view objects
	for (int i = 0; i < swapchain_image_count; i++)
	{
		fb_create_info.pAttachments = &swapchain_image_views[i];
		result = vkCreateFramebuffer(
			device,
			&fb_create_info,
			nullptr,
			&framebuffers[i]
		);
		check_vk_result(result);
	}
}

void Application::init_commands()
{
	// Create a Vulkan command pool
	const VkCommandPoolCreateInfo command_pool_create_info =
		vkinit::command_pool_create_info(
			graphics_queue_index,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
		);

	VkResult result = vkCreateCommandPool(
		device, 
		&command_pool_create_info,
		nullptr, 
		&command_pool
	);
	check_vk_result(result);

	// Allocate the main command buffer
	const VkCommandBufferAllocateInfo command_buffer_allocate_info =
		vkinit::command_buffer_allocate_info(command_pool, 1);

	result = vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		&main_command_buffer
	);
	check_vk_result(result);
}

void Application::init_sync_objects()
{
	// Create a Vulkan fence
	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext = nullptr;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult result = vkCreateFence(
		device, 
		&fence_create_info, 
		nullptr, 
		&render_fence
	);
	check_vk_result(result);

	// Create semaphores for synchronizing the rendering and presenting pipeline
	// stages
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext = nullptr;
	semaphore_create_info.flags = 0;

	result = vkCreateSemaphore(
		device, 
		&semaphore_create_info, 
		nullptr, 
		&render_semaphore
	);
	check_vk_result(result);
	result = vkCreateSemaphore(
		device,
		&semaphore_create_info,
		nullptr,
		&present_semaphore
	);
	check_vk_result(result);
}
