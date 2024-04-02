#include "matrix.h"
#include "./scaffolds.h"
#include "./operator.h"
#include "./illuminati.h"


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
        _context = new EngineContext();
        _context->setWindowExtent(window_extent);

        // Initialize SDL and create a window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _context->window_extent.width,
            _context->window_extent.height,
            window_flags
        );

        if (_window == nullptr) 
            {
                report(LOGGER::ERROR, "Reality - Failed to create SDL window ..");
                return;
            }

        // Initialize the Engine Device Context
        _initFramework();

        _context->swapchain.support = _context->querySwapChainSupport(_context->physical_device);
        _context->querySwapChainDetails();

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
        vkDeviceWaitIdle(_context->logical_device);


        report(LOGGER::INFO, "Reality - Destroying Debug Messenger ..");
        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_context->instance, _debug_messenger, nullptr); }
        
        report(LOGGER::INFO, "Reality - Destroying Gateway ..");
        destroyGateway(_gateway);

        report(LOGGER::INFO, "Reality - Destroying Context ..");
        delete _context;

        report(LOGGER::INFO, "Reality - Destroying Window ..");
        if (initialized) 
            { SDL_DestroyWindow(_window); }

        SDL_Quit();
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

    // Get Current Frame
FrameData& Reality::current_frame() { { return _context->frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }

    // Draw

void Reality::_drawFrame() 
    {
        report(LOGGER::INFO, "Matrix - Drawing Frame ..");

        vkWaitForFences(_context->logical_device, 1, &current_frame().in_flight, VK_TRUE, UINT64_MAX);

        uint32_t _image_index;
        VK_TRY(vkAcquireNextImageKHR(
                _context->logical_device, 
                _context->swapchain.instance, 
                UINT64_MAX, 
                current_frame().image_available, 
                VK_NULL_HANDLE, 
                &_image_index
            ));

        // We don't have to store these
        _context->present.wait_semaphores[0] = current_frame().image_available;
        _context->present.signal_semaphores[0] = current_frame().render_finished;
        _context->present.wait_stages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        _context->present.submit_info = getSubmitInfo(&_context->present, &current_frame().command_buffer);

        vkResetFences(_context->logical_device, 1, &current_frame().in_flight);

        VK_TRY(vkQueueSubmit(_context->queues.graphics, 1, &_context->present.submit_info, current_frame().in_flight));

        VkSwapchainKHR _swapchains[] = { _context->swapchain.instance };
        _context->present.present_info = getPresentInfoKHR(_context->present.signal_semaphores, _swapchains, &_image_index);

        VK_TRY(vkQueuePresentKHR(_context->queues.present, &_context->present.present_info));

        _frame_ct = (_frame_ct + 1) % MAX_FRAMES_IN_FLIGHT;

        return;
    }

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

                    _drawFrame();
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

        createVulkanInstance(&_context->instance);
        SDL_Vulkan_CreateSurface(_window, _context->instance, &_context->surface);
        createDebugMessenger(&_context->instance, &_debug_messenger);
        _context->createPhysicalDevice();
        _context->createLogicalDevice();

        return;
    }

void Reality::_initSwapChain(std::promise<void>& startGateway, std::future<void>& waitingForGateway, std::promise<void>& waitForFrameBuffer) 
    {
        report(LOGGER::INFO, "Matrix - Initializing SwapChain Buffers ..");

        _context->constructSwapChain();
        _context->constructImageViews();
        startGateway.set_value();
        waitingForGateway.wait();
        _context->createFrameBuffers();
        waitForFrameBuffer.set_value();
     
        return;
    }

void Reality::_initGateway(std::future<void>& startingGateway, std::promise<void>& waitForGateway) 
    {
        report(LOGGER::INFO, "Matrix - Initializing Graphics Pipeline ..");

        // abstract both of these as part of the Architect and rename _context to Architect

        startingGateway.wait();
        _context->createRenderPass();
        _gateway = constructGateway(_context);
        waitForGateway.set_value();
     
        return;
    }

void Reality::_initCommands() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Command Operator ..");

        createCommandPool(_context);
     
        return;
    }

void Reality::_initSyncStructures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures ..");

        createSyncObjects(_context);

        return;
    }


