#include "./engine.h"

#include <thread>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

    ///////////////////
    // INSTANTIATION //
    ///////////////////


NovaEngine::NovaEngine(std::string name, VkExtent2D window_extent)
    {
        report(LOGGER::INFO, "NovaEngine - Instantiating ..");

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
                report(LOGGER::ERROR, "NovaEngine - Failed to create SDL window ..");
                return;
            }


        // TODO: Wrap all of this into a subset of init functions 
        //       that use proper fences and semaphores with the following order:
        // Device()
        // Presentation()
        // Management()
        // Scene()
        // Render()

        _architect = new NovaCore(_window_extent);
        _initFramework(); // Do we want to handle this in the NovaCore?

        _architect->swapchain.support = _architect->querySwapChainSupport(_architect->physical_device);
        _architect->querySwapChainDetails();

        // Now we can construct the Swapchain
        std::promise<void> waitForSwapchain;
        std::future<void> waitingForFrameBuffer = waitForSwapchain.get_future();
        std::promise<void> startPipeline;
        std::future<void> startingPipeline = startPipeline.get_future();
        std::promise<void> waitForPipeline; 
        std::future<void> waitingForPipeline = waitForPipeline.get_future();

        std::thread _swapchain_thread(&NovaEngine::_initSwapChain, this, std::ref(startPipeline), std::ref(waitingForPipeline), std::ref(waitForSwapchain));
        std::thread _pipeline_thread(&NovaEngine::_initPipeline, this, std::ref(startingPipeline), std::ref(waitForPipeline));

        _pipeline_thread.join();
        _swapchain_thread.join();
        waitingForFrameBuffer.wait();


        // Now we can construct the Command Buffers 
        _initBuffers();
        _initSyncStructures();
        report(LOGGER::INFO, "NovaEngine - Initialized ..");
    }

NovaEngine::~NovaEngine() 
    {
        report(LOGGER::INFO, "NovaEngine - Deconstructing ..");
        vkDeviceWaitIdle(_architect->logical_device);

        if (USE_VALIDATION_LAYERS) 
            { 
                report(LOGGER::VLINE, "\t .. Destroying Debug Messenger ..");
                destroyDebugUtilsMessengerEXT(_architect->instance, _debug_messenger, nullptr); 
            }

        if (initialized) 
            {         
                report(LOGGER::VLINE, "\t .. Destroying NovaCore ..");
                delete _architect;

                report(LOGGER::VLINE, "\t .. Destroying Window ..");
                SDL_DestroyWindow(_window); 

                SDL_Quit();
            }
        
        report(LOGGER::INFO, "NovaEngine - Destroyed ..");
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

void NovaEngine::illuminate()
//void Nova::illuminate(fnManifest fnManifest)
    {
        report(LOGGER::INFO, "NovaEngine - Illuminating ..");

        SDL_Event _e;
        bool _quit = false;

        while (!_quit) {
            while (SDL_PollEvent(&_e)) 
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

                }
            
            if (_suspended) 
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
            else 
                { _architect->drawFrame(); }

            //fnManifest();
        }

        return;
    }


    //////////////////
    // INITIALIZERS //
    //////////////////

void NovaEngine::_initFramework() 
    {
        report(LOGGER::INFO, "NovaEngine - Initializing Frameworks ..");

        SDL_Vulkan_CreateSurface(_window, _architect->instance, &_architect->surface);
        createDebugMessenger(&_architect->instance, &_debug_messenger);
        _architect->createPhysicalDevice();
        _architect->createLogicalDevice();

        return;
    }

void NovaEngine::_initSwapChain(std::promise<void>& startPipeline, std::future<void>& waitingForPipeline, std::promise<void>& waitForFrameBuffer) 
    {
        report(LOGGER::INFO, "NovaEngine - Initializing SwapChain Buffers ..");

        _architect->constructSwapChain();
        _architect->constructImageViews();
        startPipeline.set_value();
        waitingForPipeline.wait();
        _architect->createCommandPool(); 
        _architect->createColorResources();
        _architect->createDepthResources();
        _architect->createFrameBuffers();
        waitForFrameBuffer.set_value();
     
        return;
    }

void NovaEngine::_initPipeline(std::future<void>& startingPipeline, std::promise<void>& waitForPipeline) 
    {
        report(LOGGER::INFO, "NovaEngine - Initializing Nova Pipeline ..");

        // abstract both of these as part of the NovaCore and rename _architect to NovaCore

        startingPipeline.wait();
        _architect->createRenderPass();
        _architect->createDescriptorSetLayout();
        _architect->constructGraphicsPipeline();
        //_architect->constructComputePipeline();
        waitForPipeline.set_value();
     
        return;
    }

void NovaEngine::_initBuffers() 
    {
        report(LOGGER::INFO, "NovaEngine - Initializing Command Operator ..");

        _architect->createTextureImage();
        _architect->createTextureImageView();
        _architect->constructTextureSampler();
        // TODO: Combine Object Phases
        _architect->constructVertexBuffer();
        _architect->constructIndexBuffer(); 
        // TODO: multithread UBO into the Presentation Phase
        _architect->constructUniformBuffer(); 
        // TODO: multithread Command Buffer to init as part of the Management Phase
        _architect->constructDescriptorPool();
        _architect->createDescriptorSets();
        _architect->createCommandBuffers();
     
        return;
    }

void NovaEngine::_initSyncStructures()
    {
        report(LOGGER::INFO, "NovaEngine - Initializing Synchronization Structures ..");

        _architect->createSyncObjects();

        return;
    }


    ///////////////////
    // RESIZE WINDOW //
    ///////////////////

inline void NovaEngine::_resizeWindow()
    {
        report(LOGGER::VERBOSE, "NovaEngine - Resizing Window ..");

        int w, h;
        SDL_Vulkan_GetDrawableSize(_window, &w, &h);
        

        _window_extent.width = w;
        _window_extent.height = h;

        _architect->setWindowExtent(_window_extent);
        _architect->framebuffer_resized = true;
        return;
    }   

