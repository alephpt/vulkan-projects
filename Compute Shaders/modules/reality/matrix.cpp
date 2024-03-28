#include "matrix.h"
#include <SDL2/SDL_vulkan.h>


    ///////////////////
    // INSTANTIATION //
    ///////////////////


Reality::Reality(std::string name, struct SDL_Window* window)
    {
        report(LOGGER::INFO, "Reality - Welcome to the Matrix ..");
        init_framework(name, window);
        init_swapchain();
        init_commands();
        init_sync_structures();
        report(LOGGER::INFO, "Reality - Matrix Initialized ..");
    }

Reality::~Reality() 
    {
        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr); }

        vkDestroyInstance(_instance, nullptr);
    }

    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::init_framework(std::string name, struct SDL_Window* window) 
    {
        report(LOGGER::INFO, "Matrix - Initializing Frameworks:");
        createVulkanInstance(&_instance);
        SDL_Vulkan_CreateSurface(window, _instance, &_surface);
        createDebugMessenger(&_instance, &_debug_messenger);
        createPhysicalDevice(_instance, &_physical_gpu, &_surface);
        createLogicalDevice(_physical_gpu, _logical_gpu, _queues, &_surface);
    }

void Reality::init_swapchain() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Buffers ..");
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
