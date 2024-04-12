#include "./graphics.h"

#include <thread>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

    ///////////////////
    // INSTANTIATION //
    ///////////////////


Graphics::Graphics(std::string name, VkExtent2D window_extent)
    {
        report(LOGGER::INFO, "Graphics - Instantiating ..");

        // Set the application name and window extent
        _application_name = name;
        _window_extent = window_extent;
        
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
                report(LOGGER::ERROR, "Graphics - Failed to create SDL window ..");
                return;
            }

        _architect = new GFXEngine(_window_extent);
        _initFramework(); // Do we want to handle this in the GFXEngine?

        _architect->swapchain.support = _architect->querySwapChainSupport(_architect->physical_device);
        _architect->querySwapChainDetails();

        // Now we can construct the Swapchain
        std::promise<void> waitForSwapchain;
        std::future<void> waitingForFrameBuffer = waitForSwapchain.get_future();
        std::promise<void> startPipeline;
        std::future<void> startingPipeline = startPipeline.get_future();
        std::promise<void> waitForPipeline; 
        std::future<void> waitingForPipeline = waitForPipeline.get_future();

        std::thread _swapchain_thread(&Graphics::_initSwapChain, this, std::ref(startPipeline), std::ref(waitingForPipeline), std::ref(waitForSwapchain));
        std::thread _pipeline_thread(&Graphics::_initPipeline, this, std::ref(startingPipeline), std::ref(waitForPipeline));

        _pipeline_thread.join();
        _swapchain_thread.join();
        waitingForFrameBuffer.wait();


        // Now we can construct the Command Buffers 
        _initBuffers();
        _initSyncStructures();
        report(LOGGER::INFO, "Graphics - Initialized ..");
    }

Graphics::~Graphics() 
    {
        report(LOGGER::INFO, "Graphics - Deconstructing ..");
        vkDeviceWaitIdle(_architect->logical_device);

        if (USE_VALIDATION_LAYERS) 
            { 
                report(LOGGER::VLINE, "\t .. Destroying Debug Messenger ..");
                destroyDebugUtilsMessengerEXT(_architect->instance, _debug_messenger, nullptr); 
            }

        if (initialized) 
            {         
                report(LOGGER::VLINE, "\t .. Destroying GFXEngineure ..");
                delete _architect;

                report(LOGGER::VLINE, "\t .. Destroying Window ..");
                SDL_DestroyWindow(_window); 

                SDL_Quit();
            }
        
        report(LOGGER::INFO, "Graphics - Destroyed ..");
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

void Graphics::illuminate()
//void Graphics::illuminate(fnManifest fnManifest)
    {
        report(LOGGER::INFO, "Graphics - Illuminating ..");

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
                                    case SDL_WINDOWEVENT_RESIZED: _resizeWindow(); break;
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

void Graphics::_initFramework() 
    {
        report(LOGGER::INFO, "Graphics - Initializing Frameworks ..");

        SDL_Vulkan_CreateSurface(_window, _architect->instance, &_architect->surface);
        createDebugMessenger(&_architect->instance, &_debug_messenger);
        _architect->createPhysicalDevice();
        _architect->createLogicalDevice();

        return;
    }

void Graphics::_initSwapChain(std::promise<void>& startPipeline, std::future<void>& waitingForPipeline, std::promise<void>& waitForFrameBuffer) 
    {
        report(LOGGER::INFO, "Graphics - Initializing SwapChain Buffers ..");

        _architect->constructSwapChain();
        _architect->constructImageViews();
        startPipeline.set_value();
        waitingForPipeline.wait();
        _architect->createFrameBuffers();
        waitForFrameBuffer.set_value();
     
        return;
    }

void Graphics::_initPipeline(std::future<void>& startingPipeline, std::promise<void>& waitForPipeline) 
    {
        report(LOGGER::INFO, "Graphics - Initializing Graphics Pipeline ..");

        // abstract both of these as part of the GFXEngine and rename _architect to GFXEngine

        startingPipeline.wait();
        _architect->createRenderPass();
        _architect->constructPipeline();
        waitForPipeline.set_value();
     
        return;
    }

void Graphics::_initBuffers() 
    {
        report(LOGGER::INFO, "Graphics - Initializing Command Operator ..");

        _architect->createCommandPool();
        _architect->constructVertexBuffer();
        _architect->constructIndexBuffer();
        _architect->createCommandBuffers();
     
        return;
    }

void Graphics::_initSyncStructures()
    {
        report(LOGGER::INFO, "Graphics - Initializing Synchronization Structures ..");

        _architect->createSyncObjects();

        return;
    }


    ///////////////////
    // RESIZE WINDOW //
    ///////////////////

inline void Graphics::_resizeWindow()
    {
        report(LOGGER::VERBOSE, "Graphics - Resizing Window ..");

        int w, h;
        SDL_Vulkan_GetDrawableSize(_window, &w, &h);
        

        _window_extent.width = w;
        _window_extent.height = h;

        _architect->setWindowExtent(_window_extent);
        _architect->framebuffer_resized = true;
        return;
    }   

