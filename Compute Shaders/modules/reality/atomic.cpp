#include "atomic.h"

#include <set>
#include <string>
#include "./virtual.h"

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

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice scanned_device, VkSurfaceKHR existing_surface, std::vector<VkQueueFamilyProperties>& _queue_families) 
    {
        report(LOGGER::DLINE, "\t .. Querying Queue Families ..");
        QueueFamilyIndices indices;
        int i = 0;

        for (const auto& _queue_family : _queue_families) {
            report(LOGGER::DLINE, "\t\tChecking Index %d", i);
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(scanned_device, i, existing_surface, &presentSupport);

            if (presentSupport) 
                { indices.present_family = i; }

            if (_queue_family.queueCount && _queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                { indices.graphics_family = i; }

            if (_queue_family.queueCount && _queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT && i != indices.graphics_family.value()) 
                { indices.compute_family = i; }

            if (indices.isComplete()) 
                { break; }

            i++;
        }

        report(LOGGER::DLINE, "\tQueue Families Found: %d", i);
        report(LOGGER::DLINE, "\tGraphics Family: %d", indices.graphics_family.value());
        report(LOGGER::DLINE, "\tPresent Family: %d", indices.present_family.value());
        report(LOGGER::DLINE, "\tCompute Family: %d", indices.compute_family.value());

        return indices;
    }

std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice scanned_device) 
    {
        report(LOGGER::DLINE, "\t .. Acquiring Queue Families ..");
        uint32_t _queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> _queue_families(_queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, _queue_families.data());

        for (int i = 0; i < _queue_families.size(); i++) 
            {
                report(LOGGER::DLINE, "\tQueue Family %d", i);
                logQueueFamilyProperties(_queue_families[i]);
            }
        return _queue_families;
    }

void logQueueFamilyProperties(VkQueueFamilyProperties& queue_family) {
    report(LOGGER::DLINE, "\t\t\tQueue Count: %d", queue_family.queueCount);

    std::string queue_name = "";

    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        { queue_name += "{ Graphics } "; }
    if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) 
        { queue_name += "{ Compute } "; }
    if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) 
        { queue_name += "{ Transfer } "; }
    if (queue_family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) 
        { queue_name += "{ Sparse Binding } "; }

    if (queue_name.empty()) 
        { queue_name = "~ Unknown ~"; }

    report(LOGGER::DLINE, "\t\t\t %s", queue_name.c_str());
}

    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& physical_device, VkSurfaceKHR& surface) 
    {
        report(LOGGER::DLINE, "\t.. Querying SwapChain Support ..");
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

        uint32_t _format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &_format_count, nullptr);

        if (_format_count != 0) 
            {
                details.formats.resize(_format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &_format_count, details.formats.data());
            }

        uint32_t _present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &_present_mode_count, nullptr);

        if (_present_mode_count != 0) 
            {
                details.present_modes.resize(_present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &_present_mode_count, details.present_modes.data());
            }

        return details;
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
        std::vector<VkQueueFamilyProperties> _queue_families = getQueueFamilies(scanned_device);
        QueueFamilyIndices queue_indices = findQueueFamilies(scanned_device, existing_surface, _queue_families);
        bool extensions_supported = checkDeviceExtensionSupport(scanned_device);

        bool swap_chain_adequate = false;
        if (extensions_supported) 
            {
                SwapChainSupportDetails swap_chain_support = querySwapChainSupport(scanned_device, existing_surface);
                swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
            }

        return queue_indices.isComplete() && extensions_supported && swap_chain_adequate;
    }
