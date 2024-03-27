#include "creation.h"
#include "genesis.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "../components/logger.h"
#include "creation.h"
#include <cassert>


Existence* _essence = nullptr;

Existence& Existence::manifest()
    {
        if (_essence == nullptr) {
            Existence essence;
            return *essence.create();
        }

        return *_essence;
    }

Existence* Existence::create() 
    {
        assert(_essence == nullptr);    // We don't ever want to re-initialize or our singleton did not work.
        _essence = this;

        report(LOGGER::INFO, "Manifesting..");

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

        return this;
    }

void Existence::materialize() 
    {
        report(LOGGER::DEBUG, "Materializing..");

        return;
    }

void Existence::actualize() 
    {
        report(LOGGER::INFO, "Actualizing..");

        SDL_Event _e;
        bool _quit = false;

        while (!_quit) {
            while (SDL_PollEvent(&_e) != 0) {
                if (_e.type == SDL_QUIT) { _quit = !_quit; }
            }

            materialize();
        }
    }

void Existence::cease() 
    {
        report(LOGGER::INFO, "Ceasing..");

        if (_initialized) {
            SDL_DestroyWindow(_window);
        }
    }