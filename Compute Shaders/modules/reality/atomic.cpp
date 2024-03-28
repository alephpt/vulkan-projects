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

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice scanned_device, VkSurfaceKHR existing_surface) 
    {
        QueueFamilyIndices indices;
        uint32_t _queue_family_count = 0;
        int i = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> _queue_families(_queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, _queue_families.data());
        report(LOGGER::VLINE, "\t\tQueue Families: %d", _queue_family_count);

        for (const auto& _queue_family : _queue_families) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(scanned_device, i, existing_surface, &presentSupport);

            if (_queue_family.queueCount > 0 && presentSupport) 
                { indices.present_family = i; }

            if (_queue_family.queueCount > 0 && _queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                { indices.graphics_family = i; }

            if (_queue_family.queueCount > 0 && _queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) 
                { indices.transfer_family = i; }

            if (_queue_family.queueCount > 0 && _queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) 
                { indices.compute_family = i; }

            if (indices.isComplete()) 
                { break; }

            i++;
        }

        return indices;
    }

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



bool deviceProvisioned(VkPhysicalDevice scanned_device, VkSurfaceKHR existing_surface)
    {
        QueueFamilyIndices queue_indices = findQueueFamilies(scanned_device, existing_surface);
        bool extensions_supported = checkDeviceExtensionSupport(scanned_device);

        return queue_indices.isComplete() && extensions_supported;
    }


    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////
