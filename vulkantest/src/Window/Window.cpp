#include "Window.h"

#include <SDL2/SDL.h>

void Window::init()
{
    window = SDL_CreateWindow(
        name,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int)extent.width,
        (int)extent.height,
        SDL_WINDOW_VULKAN |
        SDL_WINDOW_BORDERLESS
    );
}

void Window::destroy() const
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window::clicked()
{
	if (!is_clicked)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
		is_clicked = true;
	}
}

void Window::released()
{
	if (is_clicked)
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		is_clicked = false;
	}
}
