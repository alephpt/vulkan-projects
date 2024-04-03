#include "./matrix.h"

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
        _architect = new Architect();
        _architect->setWindowExtent(window_extent);

        // Initialize SDL and create a window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _architect->window_extent.width,
            _architect->window_extent.height,
            window_flags
        );

        if (_window == nullptr) 
            {
                report(LOGGER::ERROR, "Reality - Failed to create SDL window ..");
                return;
            }

        // Initialize the Engine Device Context
        _initFramework();

        _architect->swapchain.support = _architect->querySwapChainSupport(_architect->physical_device);
        _architect->querySwapChainDetails();

        // Now we can construct the Swapchain
        std::promise<void> waitForSwapchain;
        std::future<void> waitingForFrameBuffer = waitForSwapchain.get_future();
        std::promise<void> startGateway;
        std::future<void> startingGateway = startGateway.get_future();
        std::promise<void> waitForGateway; 
        std::future<void> waitingForGateway = waitForGateway.get_future();

        std::thread _swapchain_thread(&Reality::_initSwapChain, this, std::ref(startGateway), std::ref(waitingForGateway), std::ref(waitForSwapchain));
        std::thread _pipeline_thread(&Reality::_initGateway, this, std::ref(startingGateway), std::ref(waitForGateway));

        waitingForFrameBuffer.wait();

        _swapchain_thread.join();
        _pipeline_thread.join();

        // Now we can construct the Command Buffers 
        _initCommands();
        _initSyncStructures();
        report(LOGGER::INFO, "Reality - Matrix Initialized ..");
    }

Reality::~Reality() 
    {
        report(LOGGER::INFO, "Reality - Cleaning up the Matrix ..");
        vkDeviceWaitIdle(_architect->logical_device);


        report(LOGGER::INFO, "Reality - Destroying Debug Messenger ..");
        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_architect->instance, _debug_messenger, nullptr); }
        
        report(LOGGER::INFO, "Reality - Destroying Gateway ..");
        _architect->destroyGateway();

        report(LOGGER::INFO, "Reality - Destroying Context ..");
        delete _architect;

        report(LOGGER::INFO, "Reality - Destroying Window ..");
        if (initialized) 
            { SDL_DestroyWindow(_window); }

        SDL_Quit();
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

    // Main Loop

void Reality::illuminate()
//void Reality::illuminate(fnManifest fnManifest)
    {
        report(LOGGER::INFO, "Matrix - Illuminating ..");

        SDL_Event _e;
        bool _quit = false;

        printf("Illuminating ..\n");

        while (!_quit) {
            while (SDL_PollEvent(&_e) != 0) 
                {
                    if (_e.type == SDL_QUIT) { _quit = !_quit; }
                    if (_e.window.event == SDL_WINDOWEVENT_MINIMIZED) { _suspended = true; }
                    if (_e.window.event == SDL_WINDOWEVENT_RESTORED) { _suspended = false; }

                    if (_e.type == SDL_KEYDOWN) 
                        {
                            if (_e.key.keysym.sym == SDLK_ESCAPE) { _quit = !_quit; }
                        }

                    _architect->drawFrame();
                }

            if (_suspended) 
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

            //fnManifest();
        }

        return;
    }


    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::_initFramework() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Frameworks:");

        createVulkanInstance(&_architect->instance);
        SDL_Vulkan_CreateSurface(_window, _architect->instance, &_architect->surface);
        createDebugMessenger(&_architect->instance, &_debug_messenger);
        _architect->createPhysicalDevice();
        _architect->createLogicalDevice();

        return;
    }

void Reality::_initSwapChain(std::promise<void>& startGateway, std::future<void>& waitingForGateway, std::promise<void>& waitForFrameBuffer) 
    {
        report(LOGGER::INFO, "Matrix - Initializing SwapChain Buffers ..");

        _architect->constructSwapChain();
        _architect->constructImageViews();
        startGateway.set_value();
        waitingForGateway.wait();
        _architect->createFrameBuffers();
        waitForFrameBuffer.set_value();
     
        return;
    }

void Reality::_initGateway(std::future<void>& startingGateway, std::promise<void>& waitForGateway) 
    {
        report(LOGGER::INFO, "Matrix - Initializing Graphics Pipeline ..");

        // abstract both of these as part of the Architect and rename _architect to Architect

        startingGateway.wait();
        _architect->createRenderPass();
        _architect->constructGateway();
        waitForGateway.set_value();
     
        return;
    }

void Reality::_initCommands() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Command Operator ..");

        _architect->createCommandPool();
     
        return;
    }

void Reality::_initSyncStructures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures ..");

        _architect->createSyncObjects();

        return;
    }


