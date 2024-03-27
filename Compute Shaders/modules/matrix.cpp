#include "matrix.h"

#include <vector>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


    /////////////////////
    // LOCAL VARIABLES //
    /////////////////////

const bool USE_VALIDATION_LAYERS = true;
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};
const uint32_t VALIDATION_LAYER_COUNT = static_cast<uint32_t>(VALIDATION_LAYERS.size());


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData) 
    {
        report(LOGGER::DEBUG, "[VALIDATION] - %s", pCallbackData->pMessage);

        return VK_FALSE;
    }

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto destroyDebugUtilsExt = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (destroyDebugUtilsExt != nullptr) 
            { destroyDebugUtilsExt(instance, debugMessenger, pAllocator); } 
        else 
            { report(LOGGER::ERROR, "Vulkan: vkDestroyDebugUtilsMessengerEXT not available"); }
    }

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    report(LOGGER::VERBOSE, "Checking for validation layers ..");
    report(LOGGER::VLINE, "Vulkan: %d layers supported:", layerCount);
    for (const auto& layer : availableLayers) 
        { report(LOGGER::VLINE, "\t%s", layer.layerName); }

    return false;
}


    ///////////////////
    // INSTANTIATION //
    ///////////////////

Reality::Reality(std::string name) 
    {
        report(LOGGER::INFO, "Reality - Welcome to the Matrix ..");
        init_framework(name);
        init_swapchain();
        init_commands();
        init_sync_structures();
        report(LOGGER::INFO, "Reality - Matrix Initialized. Reality is ready. ..");
    }

Reality::~Reality() 
    {
        if (USE_VALIDATION_LAYERS) 
            { destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr); }

        vkDestroyInstance(_instance, nullptr);
    }


    /////////////////////
    // VULKAN INSTANCE //
    /////////////////////

static void createVulkanInstance(VkInstance *instance) 
    {
        report(LOGGER::INFO, "Matrix - Instantiating Engine ..");
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Compute Shaders";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Genesis";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        // Handle Extensions
        report(LOGGER::DEBUG, "Vulkan: Checking for extensions ..");
        
        uint32_t extension_count = 0;
        
        // This is part of the code that only gets the required extensions by SDL.
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, nullptr);

        // This is part of the code that gets all extensions supported by the system.
        // vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr); 
        

        std::vector<const char*> extensions(extension_count);

        // This code gets only the required extensions by SDL, and append the debug extension.
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, extensions.data());
        
        if (USE_VALIDATION_LAYERS){
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

        VK_TRY(vkCreateInstance(&create_info, nullptr, instance));
    }

    /////////////////////
    // DEBUG MESSENGER //
    /////////////////////

static void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger) 
    {
        if (!USE_VALIDATION_LAYERS) return;
        report(LOGGER::INFO, "Matrix - Creating Listening Agent ..");

        VkDebugUtilsMessengerCreateInfoEXT create_info = {
            sType:              VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            messageSeverity:    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            messageType:        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            pfnUserCallback: debugCallback
        };

        auto createDebugUtilsExt = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");

        if (createDebugUtilsExt != nullptr) 
            { createDebugUtilsExt(*instance, &create_info, nullptr, _debug_messenger); } 
        else 
            { report(LOGGER::ERROR, "Vulkan: vkCreateDebugUtilsMessengerEXT not available\n"); }
    }

    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::init_framework(std::string name) 
    {
        report(LOGGER::INFO, "Matrix - Initializing Frameworks ..");
        createVulkanInstance(&_instance);
        createDebugMessenger(&_instance, &_debug_messenger);
    }

void Reality::init_swapchain() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Buffers ..");
    }

void Reality::init_commands() 
    {
        report(LOGGER::INFO, "Matrix - Initializing Commands..");
    }

void Reality::init_sync_structures()
    {
        report(LOGGER::INFO, "Matrix - Initializing Synchronization Structures..");
    }
