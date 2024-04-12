#include "./atomic.h"

#include <string>
#include <unordered_set>
#include <algorithm>

    //////////////////
    //  Debug Utils //
    //////////////////

// TODO: Come back and add sanity checks in the correct places for these types of errors
static const std::unordered_set<std::string> ignorables = 
    {
        "VUID-VkFramebufferCreateInfo-flags-04533",
        "VUID-VkFramebufferCreateInfo-flags-04534",
        "VUID-VkSwapchainCreateInfoKHR-pNext-07781",
        "VUID-VkRenderPassBeginInfo-pNext-02853",
        "VUID-VkRenderPassBeginInfo-pNext-02852",
    };

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData) 
    {
        if (ignorables.find(pCallbackData->pMessageIdName) != ignorables.end()) 
            { return VK_FALSE; }
        
        report(LOGGER::INFO, "[VALIDATION] - %s", pCallbackData->pMessage);

        return VK_FALSE;
    }

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto destroyDebugUtilsExt = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (destroyDebugUtilsExt != nullptr) 
            { destroyDebugUtilsExt(instance, debugMessenger, pAllocator); } 
        else 
            { report(LOGGER::ERROR, "Vulkan: vkDestroyDebugUtilsMessengerEXT not available"); }
    }

    /////////////////////
    // DEBUG MESSENGER //
    /////////////////////

void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger) 
    {
        if (!USE_VALIDATION_LAYERS) return;
        report(LOGGER::VLINE, "\t .. Creating Listening Agent ..");

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

