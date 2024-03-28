#include "./framework.h"
#include "./matrix.h"
#include "../../components/lexicon.h"
#include <SDL2/SDL_vulkan.h>
#include <set>

    /////////////////////
    // VALIDATION LAYER //
    /////////////////////

bool checkValidationLayerSupport() 
    {
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

    /////////////////////
    // VULKAN INSTANCE //
    /////////////////////

void createVulkanInstance(VkInstance *instance) 
    {
        report(LOGGER::ILINE, "\t .. Instantiating Engine ..");
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
        report(LOGGER::VERBOSE, "Vulkan: Checking for extensions ..");
        
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

void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger) 
    {
        if (!USE_VALIDATION_LAYERS) return;
        report(LOGGER::ILINE, "\t .. Creating Listening Agent ..");

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


    /////////////////////////
    // DEVICE QUEUE FAMILY //
    /////////////////////////

bool createDeviceQueues(VkPhysicalDevice physical_gpu, VkSurfaceKHR *_surface) 
    {
        return findQueueFamilies(physical_gpu, *_surface).isComplete();
    }


    //////////////////////////
    // PHYSICAL DEVICE INFO //
    //////////////////////////

void createPhysicalDevice(VkInstance instance, VkPhysicalDevice *physical_gpu, VkSurfaceKHR *_surface) 
    {
        report(LOGGER::ILINE, "\t .. Scanning for Physical Devices ..");

        uint32_t device_count = 0;
        VK_TRY(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

        if (device_count == 0) 
            { 
                report(LOGGER::ERROR, "Vulkan: No GPUs with Vulkan support found"); 
                return;    
            }

        std::vector<VkPhysicalDevice> devices(device_count);
        VK_TRY(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

        for (const auto& device : devices) 
            {
                if (device == VK_NULL_HANDLE) 
                    { continue; }

                if (createDeviceQueues(device, _surface))
                    { 
                        VkPhysicalDeviceProperties device_properties;
                        vkGetPhysicalDeviceProperties(device, &device_properties);

                        report(LOGGER::VERBOSE, "Vulkan: Found GPU - %s", device_properties.deviceName);

                        *physical_gpu = device;
                        break; 
                    }
            }


        if (physical_gpu == VK_NULL_HANDLE) 
            { report(LOGGER::ERROR, "Vulkan: Failed to find a suitable GPU"); }
    }


    /////////////////////////
    // LOGICAL DEVICE INFO //
    /////////////////////////

void createLogicalDevice(VkPhysicalDevice physical_gpu, VkDevice logical_gpu, struct Queues& _queues, VkSurfaceKHR *_surface)
    {
        report(LOGGER::ILINE, "\t .. Creating Logical Device ..");
        QueueFamilyIndices indices = findQueueFamilies(physical_gpu, *_surface);
        VkPhysicalDeviceFeatures device_features = {};
        float queue_priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {
            indices._graphics_family.value(), 
            indices._present_family.value(), 
            indices._transfer_family.value(),
            indices._compute_family.value()
        };
        
        for (uint32_t queue_family : unique_queue_families) 
            {
                VkDeviceQueueCreateInfo queue_create_info = {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;
                queue_create_infos.push_back(queue_create_info);
            }

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = 0;

        VK_TRY(vkCreateDevice(physical_gpu, &create_info, nullptr, &logical_gpu));

        vkGetDeviceQueue(logical_gpu, indices._graphics_family.value(), 0, &_queues._graphics);
        vkGetDeviceQueue(logical_gpu, indices._present_family.value(), 0, &_queues._present);
        vkGetDeviceQueue(logical_gpu, indices._transfer_family.value(), 0, &_queues._transfer);
        vkGetDeviceQueue(logical_gpu, indices._compute_family.value(), 0, &_queues._compute);

        report(LOGGER::VERBOSE, "Matrix - Logical Device Created:");
        report(LOGGER::VLINE, "Graphics Family: %d", indices._graphics_family.value());
        report(LOGGER::VLINE, "Present Family: %d", indices._present_family.value());
        report(LOGGER::VLINE, "Logical GPU: %p", logical_gpu);
        report(LOGGER::VLINE, "Physical GPU: %p", physical_gpu);
    }


