#include "matrix.h"

#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


    /////////////////////
    // LOCAL VARIABLES //
    /////////////////////

bool use_validation_layers = true;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData) 
    {
        fmt::print(" [VALIDATION] {}\n", pCallbackData->pMessage);

        return VK_FALSE;
    }


    ///////////////////
    // INSTANTIATION //
    ///////////////////

Reality::Reality(std::string name) 
    {
        init_framework(name);
        init_swapchain();
        init_commands();
        init_sync_structures();
    }

Reality::~Reality() 
    {}


    /////////////////////
    // VULKAN INSTANCE //
    /////////////////////

static void createVulkanInstance(VkInstance *instance) 
    {
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

        uint32_t extension_count = 0;
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, nullptr);
        std::vector<const char*> extensions(extension_count);
        SDL_Vulkan_GetInstanceExtensions(nullptr, &extension_count, extensions.data());

        create_info.enabledExtensionCount = extension_count;
        create_info.ppEnabledExtensionNames = extensions.data();

        VK_TRY(vkCreateInstance(&create_info, nullptr, instance));
    }

    /////////////////////
    // DEBUG MESSENGER //
    /////////////////////

static void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger) 
    {
        if (!use_validation_layers) return;

        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        create_info.pfnUserCallback = debugCallback;

        auto createDebugUtilsExt = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");

        if (createDebugUtilsExt != nullptr) 
            { createDebugUtilsExt(*instance, &create_info, nullptr, _debug_messenger); } 
        else 
            { fmt::print(" [ERROR] Vulkan: vkCreateDebugUtilsMessengerEXT not available\n"); }
    }

    //////////////////
    // INITIALIZERS //
    //////////////////

void Reality::init_framework(std::string name) {
    // Initialize Vulkan
    createVulkanInstance(&_instance);
    createDebugMessenger(&_instance, &_debug_messenger);
}

void Reality::init_swapchain() {
}

void Reality::init_commands() {
}

void Reality::init_sync_structures()
{
}
