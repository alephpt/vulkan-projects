#pragma once
#include "./core/core.h"

#include <string>
#include <future>

// The goal of this layer of abstraction is to create a friendly user implementation for creating a graphics engine, for future projects.

// TODO: Cross Platform Support
class NovaEngine {
    public:
        bool initialized = false;

        NovaEngine(std::string, VkExtent2D);
        ~NovaEngine();

        // TODO: Determine Default Initializers 
        

        void illuminate();
        //void illuminate(fnManifest);

    private:
        std::string _application_name;
        bool _suspended = false;
        VkExtent2D _window_extent;
        struct SDL_Window* _window = nullptr;
        NovaCore* _architect;

        VkDebugUtilsMessengerEXT _debug_messenger;

        void _initFramework();
        void _initSwapChain(std::promise<void>&, std::future<void>&, std::promise<void>&);
        void _initPipeline(std::future<void>&, std::promise<void>&);
        void _initBuffers();
        void _initSyncStructures();
        void _resizeWindow();
};
