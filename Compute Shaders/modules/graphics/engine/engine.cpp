#include "engine.h"

#include <SDL2/SDL_vulkan.h>

    // Get Current Frame
FrameData& GFXEngine::current_frame() { { return frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }

    ////////////////////////
    //  INSTANCE CREATION //
    ////////////////////////

GFXEngine::GFXEngine(VkExtent2D extent) 
    {
        _blankContext();
        setWindowExtent(extent);
        createVulkanInstance();
        // TODO: Inline Initialization to be done here instead of the constructor of the top level
        // 

        // Do we want to do this, or do we want to require people to initialize these steps themselves?
        // Do we want static engine initialization, or manual dynamic state creation?

        // createPhysicalDevice();
        // createLogicalDevice();
        // surface = SDL_Vulkan_CreateSurface(_window, instance);
        // constructSwapChain();
        // constructImageViews();
        // createRenderPass();
        // createCommandPool();
        // constructVertexBuffer();
        // createCommandBuffers();
        // createSyncObjects();
        // constructPipeline();
        // setWindowExtent(extent);
    }

GFXEngine::~GFXEngine() 
    {
        report(LOGGER::INFO, "GFXEngine - Destroying Context ..");
        queues.deletion.flush();

        destroySwapChain();
        destroyUniformContext();
        vkDestroyDescriptorPool(logical_device, descriptor.pool, nullptr);
        vkDestroyDescriptorSetLayout(logical_device, descriptor.layout, nullptr);
        destroyVertexContext();
        destroyIndexContext();
        destroyCommandContext();
        destroyPipeline(graphics_pipeline);
        destroyPipeline(compute_pipeline);

        report(LOGGER::VLINE, "\t .. Destroying Pipeline and Render Pass ..");
        vkDestroyRenderPass(logical_device, render_pass, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Logical Device ..");
        vkDestroyDevice(logical_device, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Surface ..");
        vkDestroySurfaceKHR(instance, surface, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Instance ..");
        vkDestroyInstance(instance, nullptr);

        _blankContext();
    }



    /////////////////////
    // VULKAN INSTANCE //
    /////////////////////

void GFXEngine::createVulkanInstance() 
    {
        report(LOGGER::VLINE, "\t .. Instantiating Engine ..");
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Vulkan Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Vulkan Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
        };

        VkInstanceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
        };

        // Handle Extensions
        report(LOGGER::VERBOSE, "Vulkan: Checking for extensions ..");
        
        uint32_t extension_count = 0;
        
        // This is part of the code that only gets the required extensions by SDL.
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, nullptr);

        // This is part of the code that gets all extensions supported by the system.
        // vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr); 
        

        std::vector<const char*> extensions(extension_count);

        // This code gets only the required extensions by SDL, and append the debug extension.
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, extensions.data());
        
        if (USE_VALIDATION_LAYERS)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                extension_count++;
            }

        report(LOGGER::VERBOSE, "Vulkan: %d extensions supported:", extension_count);

        for (const auto& extension : extensions) 
            { report(LOGGER::VLINE, "\t%s", extension); }

        // This code gets all extensions supported by the system, but we only need the ones required by SDL. 
        //std::vector<VkExtensionProperties> extensions(extension_count);
        //vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
        
        // for (const auto& extension : extensions) 
        //     { 
        //         report(LOGGER::INFO, "\t%s", extension.extensionName); 
        //         extension_names.push_back(extension.extensionName);
        //     }

        create_info.enabledExtensionCount = extension_count;
        create_info.ppEnabledExtensionNames = extensions.data();

        // Handle Validation Layers
        if (USE_VALIDATION_LAYERS && !checkValidationLayerSupport()) 
            {
                create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYER_COUNT);
                create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

                VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
                    sType:              VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    messageSeverity:    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                    messageType:        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                    pfnUserCallback: debugCallback   
                    
                };

                create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
            } 
        else 
            { 
                create_info.enabledLayerCount = 0; 
                create_info.pNext = nullptr;
            }

        VK_TRY(vkCreateInstance(&create_info, nullptr, &instance));
    }


    //////////////////////////////
    //  BLANK CONTEXT CREATION  //
    //////////////////////////////

static Queues initQueues() 
    {
        return {
                .graphics = VK_NULL_HANDLE,
                .present = VK_NULL_HANDLE,
                .transfer = VK_NULL_HANDLE,
                .compute = VK_NULL_HANDLE,
                .deletion = {},
                .xfr = {
                    .pool = VK_NULL_HANDLE,
                    .buffer = VK_NULL_HANDLE
                },
                .cmp = {
                    .pool = VK_NULL_HANDLE,
                    .buffer = VK_NULL_HANDLE
                },
                .families = {},
                .indices = {},
                .priorities = {}
            };
    }

static SwapChainSupportDetails initSwapChainSupport() 
    {
        return {
                .capabilities = {},
                .formats = {},
                .present_modes = {}
            };
    }

static SwapChainDetails initSwapChainDetails() 
    {
        return {
                .surface_format = {},
                .present_mode = {},
                .extent = {}
            };
    }

static SwapChainContext initSwapchain() {
    return {
            .instance = VK_NULL_HANDLE,
            .images = {},
            .image_views = {},
            .framebuffers = {},
            .format = VK_FORMAT_UNDEFINED,
            .extent = {0, 0},
            .support = initSwapChainSupport(),
            .details = initSwapChainDetails()
        };
}


void GFXEngine::_blankContext() 
    {
        report(LOGGER::INFO, "GFXEngine - No Context ..");

        instance = VK_NULL_HANDLE;
        physical_device = VK_NULL_HANDLE;
        logical_device = VK_NULL_HANDLE;
        surface = VK_NULL_HANDLE;
        render_pass = VK_NULL_HANDLE;
        queues = initQueues();
        swapchain = initSwapchain();
        present = {};
        graphics_pipeline = nullptr;
        compute_pipeline = nullptr;
        vertex = {};
        index = {};
        uniform = {};
        uniform_data = {};
    }

// This should not be done like this
void GFXEngine::setWindowExtent(VkExtent2D extent) 
    {
        swapchain.extent = extent;
        //swapchain.details.extent = extent;
        framebuffer_resized = true;
    }


    /////////////
    // LOGGING //
    /////////////

void GFXEngine::logQueues() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Queues ..");
        report(LOGGER::DLINE, "\t\tFamilies: %d", queues.families.size());
        report(LOGGER::DLINE, "\t\tPresent Family Index: %d", queues.indices.present_family.value());
        report(LOGGER::DLINE, "\t\tPresent: %p", queues.present);
        report(LOGGER::DLINE, "\t\tGraphics Family Index: %d", queues.indices.graphics_family.value());
        report(LOGGER::DLINE, "\t\tGraphics: %p", queues.graphics);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                report(LOGGER::DLINE, "\t\t\tCommand Pool (Graphics %d): %p", i, frames[i].cmd.pool);
                report(LOGGER::DLINE, "\t\t\tCommand Buffer (Graphics %d): %p", i, frames[i].cmd.buffer);
            }
        report(LOGGER::DLINE, "\t\tTransfer Family Index: %d", queues.indices.transfer_family.value());
        report(LOGGER::DLINE, "\t\tTransfer: %p", queues.transfer);
        report(LOGGER::DLINE, "\t\tCommand Pool (Transfer): %p", queues.xfr.pool);
        report(LOGGER::DLINE, "\t\tCommand Buffer (Transfer): %p", queues.xfr.buffer);
        report(LOGGER::DLINE, "\t\tCompute Family Index: %d", queues.indices.compute_family.value());
        report(LOGGER::DLINE, "\t\tCompute: %p", queues.compute);
        report(LOGGER::DLINE, "\t\tCommand Pool (Compute): %p", queues.cmp.pool);
        report(LOGGER::DLINE, "\t\tCommand Buffer (Compute): %p", queues.cmp.buffer);
        report(LOGGER::DLINE, "\t\tPriorities: %d", queues.priorities.size());
    }

void GFXEngine::logFrameData()
    {
        report(LOGGER::DEBUG, "\t .. Logging Frame Data ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                report(LOGGER::DLINE, "\t\tFrame %d", i);
                report(LOGGER::DLINE, "\t\t\tImage Available: %p", frames[i].image_available);
                report(LOGGER::DLINE, "\t\t\tRender Finished: %p", frames[i].render_finished);
                report(LOGGER::DLINE, "\t\t\tTransfer Finished: %p", frames[i].transfer_finished);
                report(LOGGER::DLINE, "\t\t\tComputer Finished: %p", frames[i].compute_finished);
                report(LOGGER::DLINE, "\t\t\tIn Flight: %p", frames[i].in_flight);
            }
    
    }

void GFXEngine::logSwapChain() 
    {
        report(LOGGER::DEBUG, "\t .. Logging SwapChain ..");
        report(LOGGER::DLINE, "\t\tSwapchain: %p", swapchain.instance);
        report(LOGGER::DLINE, "\t\tImage Count: %d", swapchain.images.size());
        report(LOGGER::DLINE, "\t\tImage Views: %d", swapchain.image_views.size());
        report(LOGGER::DLINE, "\t\tFramebuffers: %d", swapchain.framebuffers.size());
        report(LOGGER::DLINE, "\t\tImage Format: %d", swapchain.format);
        report(LOGGER::DLINE, "\t\tSupport: %d Formats", swapchain.support.formats.size());
        report(LOGGER::DLINE, "\t\tSupport: %d Present Modes", swapchain.support.present_modes.size());
        report(LOGGER::DLINE, "\t\tDetails: %d Formats", swapchain.details.surface_format.format);
        report(LOGGER::DLINE, "\t\tDetails: %d Present Modes", swapchain.details.present_mode);
        report(LOGGER::DLINE, "\t\tExtent: %d x %d", swapchain.extent.width, swapchain.extent.height);
    }

void GFXEngine::log() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Context ..");
        report(LOGGER::DLINE, "\t\tInstance: %p", instance);
        report(LOGGER::DLINE, "\t\tPhysical Device: %p", physical_device);
        report(LOGGER::DLINE, "\t\tLogical Device: %p", logical_device);
        report(LOGGER::DLINE, "\t\tSurface: %p", surface);
        logQueues();
        logSwapChain();
        report(LOGGER::DLINE, "\t\tRender Pass: %p", render_pass);
        report(LOGGER::DLINE, "\t\tPresent Info: %p", &present);
        logFrameData();
    }
