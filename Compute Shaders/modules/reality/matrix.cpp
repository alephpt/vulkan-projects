#include "matrix.h"
#include "./scaffolds.h"
#include "./virtual.h"
#include "./veil.h"
#include "./operator.h"

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
        _context.window_extent = window_extent;

        // Initialize SDL and create a window
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _context.window_extent.width,
            _context.window_extent.height,
            window_flags
        );

        if (_window == nullptr) 
            {
                report(LOGGER::ERROR, "Reality - Failed to create SDL window ..");
                return;
            }

        // Initialize the Engine Device Context
        _initFramework();

        // We need to multithread the Swapchain and Pipeline Instantiation
        _context.swapchain.support = querySwapChainSupport(_context.physical_device, _context.surface);
        _context.swapchain.details = querySwapChainDetails(_context.swapchain.support, _context.window_extent);

        // Now we can construct the Swapchain
        std::promise<void> waitForSwapchain;
        std::future<void> waitingForFrameBuffer = waitForSwapchain.get_future();
        std::promise<void> waitForGateway; 
        std::future<void> waitingForGateway = waitForGateway.get_future();

        std::thread _swapchain_thread(&Reality::_initSwapChain, this, std::move(waitingForGateway), std::move(waitForSwapchain));
        std::thread _pipeline_thread(&Reality::_initGateway, this, std::move(waitForGateway));
        waitingForFrameBuffer.wait();

        _swapchain_thread.join();
        _pipeline_thread.join();

        // We need to multithread the Swapchain and Pipeline Instantiation

        // Now we can construct the Command Buffers 
        _initCommands();
        _initSyncStructures();
        report(LOGGER::INFO, "Reality - Matrix Initialized ..");
    }

Reality::~Reality() 
    {
        report(LOGGER::INFO, "Reality - Cleaning up the Matrix ..");
        vkDeviceWaitIdle(_context.logical_device);

        destroySwapChain(&_context);

        report(LOGGER::INFO, "Reality - Destroying Semaphores, Fences and Command Pools ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(_context.logical_device, _context.frames[i].image_available, nullptr);
                vkDestroySemaphore(_context.logical_device, _context.frames[i].render_finished, nullptr);
                vkDestroyFence(_context.logical_device, _context.frames[i].in_flight, nullptr);
                vkDestroyCommandPool(_context.logical_device, _context.frames[i].command_pool, nullptr);
            }

        report(LOGGER::INFO, "Reality - Destroying Gateway and Render Pass ..");
        destroyGateway(_gateway);
        vkDestroyRenderPass(_context.logical_device, _context.render_pass, nullptr);

        report(LOGGER::INFO, "Reality - Destroying Logical Device ..");
        vkDestroyDevice(_context.logical_device, nullptr);

        report(LOGGER::INFO, "Reality - Destroying Debug Messenger ..");
        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_context.instance, _debug_messenger, nullptr); }

        report(LOGGER::INFO, "Reality - Destroying Surface ..");
        vkDestroySurfaceKHR(_context.instance, _context.surface, nullptr);

        report(LOGGER::INFO, "Reality - Destroying Instance ..");
        vkDestroyInstance(_context.instance, nullptr);
        
        report(LOGGER::INFO, "Reality - Destroying Window ..");
        if (initialized) 
            { SDL_DestroyWindow(_window); }

        SDL_Quit();
    }


    /////////////////////////
    // TOP LEVEL FUNCTIONS //
    /////////////////////////

    // Get Current Frame

FrameData& Reality::current_frame() { { return _context.frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }

    // Draw

void Reality::_drawFrame() 
    {
        report(LOGGER::INFO, "Matrix - Drawing Frame ..");

        vkWaitForFences(_context.logical_device, 1, &current_frame().in_flight, VK_TRUE, UINT64_MAX);

        uint32_t _image_index;
        VkResult _result = vkAcquireNextImageKHR(
            _context.logical_device, _context.swapchain.instance, UINT64_MAX, 
            current_frame().image_available, VK_NULL_HANDLE, &_image_index
        );

        if (_result == VK_ERROR_OUT_OF_DATE_KHR) 
            {
                report(LOGGER::ERROR, "Matrix - Swapchain out of date ..");
                return;
            }

        if (_result != VK_SUCCESS && _result != VK_SUBOPTIMAL_KHR) 
            {
                report(LOGGER::ERROR, "Matrix - Failed to acquire next image ..");
                return;
            }

        VkSemaphore _wait_semaphores[] = { current_frame().image_available };
        VkSemaphore _signal_semaphores[] = { current_frame().render_finished };
        VkPipelineStageFlags _wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo _submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = _wait_semaphores,
            .pWaitDstStageMask = _wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &current_frame().command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = _signal_semaphores
        };

        vkResetFences(_context.logical_device, 1, &current_frame().in_flight);

        VK_TRY(vkQueueSubmit(_context.queues.graphics, 1, &_submit_info, current_frame().in_flight));

        VkSwapchainKHR _swapchains[] = { _context.swapchain.instance };
        VkPresentInfoKHR _present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = _signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = _swapchains,
            .pImageIndices = &_image_index
        };

        VK_TRY(vkQueuePresentKHR(_context.queues.present, &_present_info));

        _frame_ct = (_frame_ct + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // Main Loop

void Reality::illuminate()
//void Reality::illuminate(fnManifest fnManifest)
    {
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
    }

    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::_initFramework() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Frameworks:");
        createVulkanInstance(&_context.instance);
        SDL_Vulkan_CreateSurface(_window, _context.instance, &_context.surface);
        createDebugMessenger(&_context.instance, &_debug_messenger);
        createPhysicalDevice(&_context);
        createLogicalDevice(&_context);
    }

void Reality::_initSwapChain(std::future<void>&& waitingForGateway, std::promise<void>&& waitForFrameBuffer) 
    {
        report(LOGGER::INFO, "Matrix - Initializing SwapChain Buffers ..");
        constructSwapChain(_context.swapchain.details, _context.swapchain.support, &_context);
        constructImageViews(&_context);
        waitingForGateway.wait();
        createFrameBuffers(&_context);
        waitForFrameBuffer.set_value();
    }

void Reality::_initGateway(std::promise<void>&& waitForGateway) 
    {
        report(LOGGER::INFO, "Matrix - Initializing Graphics Pipeline ..");
        createRenderPass(&_context);
        constructGateway(&_context, _gateway);
        waitForGateway.set_value();
    }

void Reality::_initCommands() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Command Operator ..");

        createCommandPool(&_context);
    }

void Reality::_initSyncStructures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures ..");

        createSyncObjects(&_context);
    }


