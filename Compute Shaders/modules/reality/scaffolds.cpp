#include "./scaffolds.h"
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
        report(LOGGER::DLINE, "\t .. Instantiating Engine ..");
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

        VK_TRY(vkCreateInstance(&create_info, nullptr, instance));
    }


    /////////////////////
    // DEBUG MESSENGER //
    /////////////////////

void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger) 
    {
        if (!USE_VALIDATION_LAYERS) return;
        report(LOGGER::DLINE, "\t .. Creating Listening Agent ..");

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


    //////////////////////////
    // PHYSICAL DEVICE INFO //
    //////////////////////////

void createPhysicalDevice(EngineContext *context) 
    {
        report(LOGGER::DLINE, "\t .. Scanning for Physical Devices ..");

        uint32_t device_count = 0;
        VK_TRY(vkEnumeratePhysicalDevices(context->instance, &device_count, nullptr));

        if (device_count == 0) 
            { 
                report(LOGGER::ERROR, "Vulkan: No GPUs with Vulkan support found"); 
                return;    
            }

        std::vector<VkPhysicalDevice> devices(device_count);
        VK_TRY(vkEnumeratePhysicalDevices(context->instance, &device_count, devices.data()));

        for (const auto& device : devices) 
            {
                VkPhysicalDeviceProperties device_properties;
                vkGetPhysicalDeviceProperties(device, &device_properties);
                report(LOGGER::VLINE, "\tScanning Device: %p - %s", device, device_properties.deviceName);
                if (device == VK_NULL_HANDLE) 
                    { continue; }

                if (deviceProvisioned(device, context))
                    { 
                        report(LOGGER::VLINE, "\tUsing Device: %s", device_properties.deviceName);

  
                        context->physical_device = device;
                        break; 
                    }
            }


        if (context->physical_device == VK_NULL_HANDLE) 
            { report(LOGGER::ERROR, "Vulkan: Failed to find a suitable GPU"); }
    }


    /////////////////////////
    // LOGICAL DEVICE INFO //
    /////////////////////////

void createLogicalDevice(EngineContext *context)
    {
        report(LOGGER::DLINE, "\t .. Creating Logical Device ..");
        VkPhysicalDeviceFeatures _device_features = {};

        std::vector<VkDeviceQueueCreateInfo> _queue_create_infos;
        std::set<uint32_t> _unique_queue_families = {
            context->queues.indices.graphics_family.value(), 
            context->queues.indices.present_family.value(),
            context->queues.indices.compute_family.value()
        };

        report(LOGGER::VLINE, "\t .. Creating Queue Family ..");
        for (uint32_t _queue_family : _unique_queue_families) 
            {
                report(LOGGER::VLINE, "\t\tQueue Family: %d", _queue_family);
                report(LOGGER::VLINE, "\t\t\tQueue Count: %d", context->queues.families[_queue_family].queueCount);
                if (_queue_family == -1) { continue; } // if the queue family is not supported

                // resize the queue priorities to match the number of queues in the family
                

                VkDeviceQueueCreateInfo queue_create_info = {
                    sType: VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    queueFamilyIndex: _queue_family,
                    queueCount: context->queues.families[_queue_family].queueCount,
                    pQueuePriorities: context->queues.priorities[_queue_family].data()
                };

                _queue_create_infos.push_back(queue_create_info);
            }

        VkDeviceCreateInfo create_info = {
            sType: VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            queueCreateInfoCount: static_cast<uint32_t>(_queue_create_infos.size()),
            pQueueCreateInfos: _queue_create_infos.data(),
            pEnabledFeatures: &_device_features,
        };

        create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

        VK_TRY(vkCreateDevice(context->physical_device, &create_info, nullptr, &context->logical_device));

        vkGetDeviceQueue(context->logical_device, context->queues.indices.graphics_family.value(), 0, &context->queues.graphics);
        vkGetDeviceQueue(context->logical_device, context->queues.indices.present_family.value(), 0, &context->queues.present);
        vkGetDeviceQueue(context->logical_device, context->queues.indices.compute_family.value(), 0, &context->queues.compute);

        logContext(context);
    }
