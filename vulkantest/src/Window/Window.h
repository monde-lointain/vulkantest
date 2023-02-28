#pragma once

#include <memory>

#include <vulkan/vulkan_core.h>

struct SDL_Renderer;
struct SDL_Window;
struct Viewport;

class Window
{
public:
	void init();
	void destroy() const;

	void clicked();
	void released();

	SDL_Window* window = nullptr;
	VkExtent2D extent = { 1920, 1080 };
	VkRect2D scissor = { { 0, 0 }, extent };
	const char* name = "Vulkan Renderer";

private:
	bool is_clicked = false;
};

