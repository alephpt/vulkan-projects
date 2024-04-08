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
        report(LOGGER::INFO, "Matrix - Welcome to Reality ..");

        // Set the application name and window extent
        _application_name = name;
        _window_extent = window_extent;
        _architect = new Architect();
        _architect->setWindowExtent(_window_extent);

        // Initialize SDL and create a window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

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
                report(LOGGER::ERROR, "Matrix - Failed to create SDL window ..");
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

        _pipeline_thread.join();
        _swapchain_thread.join();
        waitingForFrameBuffer.wait();


        // Now we can construct the Command Buffers 
        _initBuffers();
        _initSyncStructures();
        report(LOGGER::INFO, "Matrix - Initialized ..");
    }

Reality::~Reality() 
    {
        report(LOGGER::INFO, "Matrix - Deconstructing ..");
        vkDeviceWaitIdle(_architect->logical_device);

        if (USE_VALIDATION_LAYERS) 
            { 
                report(LOGGER::DLINE, "\t .. Destroying Debug Messenger ..");
                destroyDebugUtilsMessengerEXT(_architect->instance, _debug_messenger, nullptr); 
            }

        if (initialized) 
            {         
                report(LOGGER::DLINE, "\t .. Destroying Architecture ..");
                delete _architect;

                report(LOGGER::DLINE, "\t .. Destroying Window ..");
                SDL_DestroyWindow(_window); 

                SDL_Quit();
            }
        
        report(LOGGER::INFO, "Matrix - Destroyed ..");
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

void Reality::illuminate()
//void Reality::illuminate(fnManifest fnManifest)
    {
        report(LOGGER::INFO, "Matrix - Illuminating ..");

        SDL_Event _e;
        bool _quit = false;

        while (!_quit) {
            while (SDL_PollEvent(&_e) != 0) 
                {
                    if (_e.type == SDL_QUIT) { _quit = !_quit; }
                    
                    if (_e.type == SDL_KEYDOWN) 
                        { if (_e.key.keysym.sym == SDLK_ESCAPE) { _quit = !_quit; } }

                    if (_e.type == SDL_WINDOWEVENT) 
                        { 
                            switch (_e.window.event) 
                                {
                                    case SDL_WINDOWEVENT_MINIMIZED: _suspended = true; break;
                                    case SDL_WINDOWEVENT_RESTORED: _suspended = false; break;
                                    case SDL_WINDOWEVENT_RESIZED: _resizeWindow(_e.window.data1, _e.window.data2); break;
                                }
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
        report(LOGGER::INFO, "Matrix - Initializing Frameworks ..");

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

void Reality::_initBuffers() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Command Operator ..");

        _architect->createCommandPool();
        _architect->constructVertexBuffer();
        _architect->createCommandBuffers();
     
        return;
    }

void Reality::_initSyncStructures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures ..");

        _architect->createSyncObjects();

        return;
    }


    ///////////////////
    // RESIZE WINDOW //
    ///////////////////

inline void Reality::_resizeWindow(int w, int h)
    {
        report(LOGGER::INFO, "Matrix - Resizing Window ..");
        _window_extent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
        _architect->setWindowExtent(_window_extent);
        _architect->framebuffer_resized = true;
        return;
    }   

