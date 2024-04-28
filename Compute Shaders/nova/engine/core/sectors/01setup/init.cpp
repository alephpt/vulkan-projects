#include "../../core.h"

#include <SDL2/SDL_vulkan.h>

    ////////////////////////
    //  INSTANCE CREATION //
    ////////////////////////

NovaCore::NovaCore(VkExtent2D extent) 
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

NovaCore::~NovaCore() 
    {
        report(LOGGER::INFO, "NovaCore - Destroying Context ..");

        destroySwapChain();
        destroyImageContext();
        queues.deletion.flush();
        destroyUniformContext();
        vkDestroyDescriptorPool(logical_device, descriptor.pool, nullptr);
        vkDestroyDescriptorSetLayout(logical_device, descriptor.layout, nullptr);
        destroyVertexContext();
        destroyIndexContext();
        destroyCommandContext();
        destroyPipeline(graphics_pipeline);
        //destroyPipeline(compute_pipeline);

        report(LOGGER::VLINE, "\t .. Destroying Pipeline and Render Pass.");
        vkDestroyRenderPass(logical_device, render_pass, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Logical Device.");
        vkDestroyDevice(logical_device, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Surface.");
        vkDestroySurfaceKHR(instance, surface, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Instance.");
        vkDestroyInstance(instance, nullptr);

        _blankContext();
    }



    /////////////////////
    // VULKAN INSTANCE //
    /////////////////////

void NovaCore::createVulkanInstance() 
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
                .surface = {},
                .present_mode = {},
            };
    }

static SwapChainContext initSwapchain() {
    return {
            .instance = VK_NULL_HANDLE,
            .images = {},
            .image_views = {},
            .framebuffers = {},
            .support = initSwapChainSupport(),
            .details = initSwapChainDetails()
        };
}

static inline ImageContext _initImageContext() 
    {
        return {
                .image = VK_NULL_HANDLE,
                .memory = VK_NULL_HANDLE,
                .view = VK_NULL_HANDLE,
                .sampler = VK_NULL_HANDLE
            };
    }

void NovaCore::_blankContext() 
    {
        report(LOGGER::INFO, "NovaCore - No Context ..");

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
        color = _initImageContext();
        depth = _initImageContext();
        texture = _initImageContext();
        descriptor = {
            .layout = VK_NULL_HANDLE,
            .pool = VK_NULL_HANDLE,
            .sets = {}
        };
    }

// This should not be done like this
void NovaCore::setWindowExtent(VkExtent2D extent) 
    {
        swapchain.details.extent = extent;
        framebuffer_resized = true;
    }
