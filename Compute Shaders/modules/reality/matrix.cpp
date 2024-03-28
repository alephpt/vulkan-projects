#include "matrix.h"
#include "./scaffolds.h"
#include "./virtual.h"

#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


    ///////////////////
    // INSTANTIATION //
    ///////////////////


Reality::Reality(std::string name, VkExtent2D window_extent)
    {
        report(LOGGER::INFO, "Reality - Welcome to the Matrix ..");

        // Set the application name and window extent
        _application_name = name;
        _window_extent = window_extent;

        // Initialize SDL and create a window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _window_extent.width,
            _window_extent.height,
            window_flags
        );

        if (_window == nullptr) 
            {
                report(LOGGER::ERROR, "Reality - Failed to create SDL window ..");
                return;
            }

        // Initialize the Vulkan Framework
        init_framework();
        init_swapchain();
        init_commands();
        init_sync_structures();
        report(LOGGER::INFO, "Reality - Matrix Initialized ..");
    }

Reality::~Reality() 
    {
        report(LOGGER::INFO, "Reality - Cleaning up the Matrix ..");
        vkDestroySwapchainKHR(_logical_gpu, _swapchain, nullptr);
        vkDestroyDevice(_logical_gpu, nullptr);

        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr); }

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);


        if (initialized) {
            SDL_DestroyWindow(_window);
        }

        SDL_Quit();
    }

void Reality::illuminate()//fnManifest manifest)
    {
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

            //manifest();
        }
    }

    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::init_framework() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Frameworks:");
        createVulkanInstance(&_instance);
        SDL_Vulkan_CreateSurface(_window, _instance, &_surface);
        createDebugMessenger(&_instance, &_debug_messenger);
        createPhysicalDevice(_instance, &_physical_gpu, &_surface);
        createLogicalDevice(_physical_gpu, _logical_gpu, _queues, &_surface);
    }

void Reality::init_swapchain() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Buffers ..");
        SwapChainSupportDetails _swapchain_support = querySwapChainSupport(_physical_gpu, _surface);
        SwapChainDetails _swapchain_details = querySwapChainDetails(_swapchain_support, _window_extent);
    }

void Reality::init_commands() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Commands ..");
    }

void Reality::init_sync_structures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures ..");
    }

FrameData &Reality::_current_frame() { { return _frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }
