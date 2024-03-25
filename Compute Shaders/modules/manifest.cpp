#include "./manifest.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "./genesis.h"

void Existence::manifest() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    _window = SDL_CreateWindow(
        "Compute Shaders",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        _window_extent.width,
        _window_extent.height,
        window_flags
    );

    _initialized = true;
    return;
}

void Existence::materialize() {
    return;
}

void Existence::actualize() {
    SDL_Event _e;
    bool _quit = false;

    while (!_quit) {
        while (SDL_PollEvent(&_e) != 0) {
            if (_e.type == SDL_QUIT) { _quit = !_quit; }
        }

        materialize();
    }
}

void Existence::cease() {
    if (_initialized) {
        SDL_DestroyWindow(_window);
    }
}