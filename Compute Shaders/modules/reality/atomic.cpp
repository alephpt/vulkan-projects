#include "atomic.h"

#include <set>
#include <string>

    //////////////////
    //  Debug Utils //
    //////////////////

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData) 
    {
        report(LOGGER::VERBOSE, "[VALIDATION] - %s", pCallbackData->pMessage);

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

    ////////////////////
    //  Device Queues //
    ////////////////////

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) 
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        int i = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport) 
                { indices._present_family = i; }

            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                { indices._graphics_family = i; }

            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) 
                { indices._transfer_family = i; }

            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 
                { indices._compute_family = i; }

            if (indices.isComplete()) 
                { break; }

            i++;
        }

        return indices;
    }


    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

    /////////////////////////////////
    // DEVICE MANAGEMENT UTILITIES //
    /////////////////////////////////


static bool checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

        for (const auto& extension : availableExtensions) 
            { requiredExtensions.erase(extension.extensionName); }

        return requiredExtensions.empty();
    }



bool deviceProvisioned(VkPhysicalDevice physical_gpu, VkSurfaceKHR *_surface) 
    {
        QueueFamilyIndices queue_indices = findQueueFamilies(physical_gpu, *_surface);
        bool extensions_supported = checkDeviceExtensionSupport(physical_gpu);

        return queue_indices.isComplete() && extensions_supported;
    }