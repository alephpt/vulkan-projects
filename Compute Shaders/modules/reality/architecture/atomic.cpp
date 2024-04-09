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
