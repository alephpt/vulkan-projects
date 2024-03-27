#include "creation.h"
#include "genesis.h"
#include "../components/logger.h"

#include <cassert>
#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


Existence* _essence = nullptr;

// Singleton to ensure only one instance of Existence is created.
Existence& Existence::manifest()
    {
        report(LOGGER::INFO, "Existence - Conjuring Manifestation ..");
        
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

        report(LOGGER::INFO, "Existence - Creating Manifestation ..");

        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            _application_name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _window_extent.width,
            _window_extent.height,
            window_flags
        );

        report(LOGGER::INFO, "Existence - Constructing Reality ..");
        // Handles Vulkan Rendering Engine and Compute Buffers
        _actuality = new Reality(_application_name);

        // Initialize the Reality with Genesis 

        report(LOGGER::INFO, "Existence - Reality Complete ..");

        _initialized = true;

        return this;
    }

void Existence::materialize() 
    {
        report(LOGGER::VERBOSE, "Existence - Materializing ..");

        return;
    }

void Existence::actualize() 
    {
        report(LOGGER::INFO, "Existence - Actualizing ..");

        SDL_Event _e;
        bool _quit = false;

        while (!_quit) {
            while (SDL_PollEvent(&_e) != 0) {
                if (_e.type == SDL_QUIT) { _quit = !_quit; }
                if (_e.window.event == SDL_WINDOWEVENT_MINIMIZED) { _suspended = true; }
                if (_e.window.event == SDL_WINDOWEVENT_RESTORED) { _suspended = false; }
            }

            if (_suspended) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            materialize();
        }
    }

void Existence::cease() 
    {
        report(LOGGER::INFO, "Existence - Ceasing ..");

        if (_initialized) {
            SDL_DestroyWindow(_window);
        }

        _essence = nullptr;
    }