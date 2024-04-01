#include "atomic.h"

#include <set>
#include <string>
#include "./virtual.h"


void logContext(EngineContext *context) 
    {
        report(LOGGER::VLINE, "\t .. Logging Context ..");
        report(LOGGER::VLINE, "\t\tInstance: %p", context->instance);
        report(LOGGER::VLINE, "\t\tPhysical Device: %p", context->physical_device);
        report(LOGGER::VLINE, "\t\tLogical Device: %p", context->logical_device);
        report(LOGGER::VLINE, "\t\tSurface: %p", context->surface);
        report(LOGGER::VLINE, "\t\tQueue Families: %d", context->queues.families.size());
        report(LOGGER::VLINE, "\t\tQueue Indices: %d", context->queues.priorities.size());
    }

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


static inline void setQueueFamilyProperties(EngineContext* context, int i) {
    VkQueueFamilyProperties* queue_family = &context->queues.families[i];
    std::string queue_name = "";

    if (queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        { 
            queue_name += "{ Graphics } "; 
            context->queues.indices.graphics_family = i;
            context->queues.priorities.push_back(std::vector<float>(queue_family->queueCount, 1.0f));
            report(LOGGER::VLINE, "\t\tGraphics Family Set.");
        }
    if (queue_family->queueFlags & VK_QUEUE_COMPUTE_BIT) 
        { 
            queue_name += "{ Compute } "; 
            if (context->queues.indices.graphics_family.value() != i) 
                {
                    context->queues.indices.compute_family = i;
                    context->queues.priorities.push_back(std::vector<float>(queue_family->queueCount, 1.0f));
                    report(LOGGER::VLINE, "\t\tCompute Family Set.");
                }
        }
    if (queue_family->queueFlags & VK_QUEUE_TRANSFER_BIT) 
        { 
            queue_name += "{ Transfer } "; 
        }
    if (queue_family->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) 
        { 
            queue_name += "{ Sparse Binding } "; 
        }

    if (queue_name.empty()) 
        { queue_name = "~ Unknown ~"; }

    report(LOGGER::VLINE, "\t\t\tQueue Count: %d", queue_family->queueCount);
    report(LOGGER::VLINE, "\t\t\t %s", queue_name.c_str());
}

static inline void getQueueFamilies(VkPhysicalDevice scanned_device, EngineContext *context) 
    {
        report(LOGGER::VLINE, "\t .. Acquiring Queue Families ..");
        uint32_t _queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, nullptr);
        context->queues.families.resize(_queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, context->queues.families.data());

        for (int i = 0; i < context->queues.families.size(); i++) 
            {
                report(LOGGER::VLINE, "\t\tQueue Family %d", i);

                // If we haven't found a present family yet, we'll take the first one we find
                if (context->queues.indices.present_family.value() == -1){
                    VkBool32 _present_support = false;  
                    vkGetPhysicalDeviceSurfaceSupportKHR(scanned_device, i, context->surface, &_present_support);

                    if (_present_support) 
                        { 
                            context->queues.indices.present_family = i; 
                            report(LOGGER::VLINE, "\t\tPresent Family Set.");    
                        }
                }

                setQueueFamilyProperties(context, i);
            }
    }


    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& physical_device, VkSurfaceKHR& surface) 
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Support ..");
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

bool deviceProvisioned(VkPhysicalDevice scanned_device, EngineContext *context)
    {
        getQueueFamilies(scanned_device, context);
        bool extensions_supported = checkDeviceExtensionSupport(scanned_device);

        bool swap_chain_adequate = false;
        if (extensions_supported) 
            {
                SwapChainSupportDetails swap_chain_support = querySwapChainSupport(scanned_device, context->surface);
                swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
            }

        return context->queues.indices.isComplete() && extensions_supported && swap_chain_adequate;
    }