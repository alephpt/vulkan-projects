#include "../engine.h"

#include <set>
#include <string>


    //////////////////////
    // VALIDATION LAYER //
    //////////////////////

bool GFXEngine::checkValidationLayerSupport() 
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


    ////////////////////
    //  Device Queues //
    ////////////////////

void GFXEngine::setQueueFamilyProperties(unsigned int i) {
    VkQueueFamilyProperties* queue_family = &queues.families[i];
    std::string queue_name = "";

    if (queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        { 
            queue_name += "{ Graphics } "; 
            queues.indices.graphics_family = i;
            queues.priorities.push_back(std::vector<float>(queue_family->queueCount, 1.0f));
            report(LOGGER::VLINE, "\t\tGraphics Family Set.");
        }

    if (queue_family->queueFlags & VK_QUEUE_COMPUTE_BIT) 
        { 
            queue_name += "{ Compute } "; 

            if (queues.indices.graphics_family.value() != i) 
                {
                    queues.indices.compute_family = i;
                    queues.priorities.push_back(std::vector<float>(queue_family->queueCount, 1.0f));
                    report(LOGGER::VLINE, "\t\tCompute Family Set.");
                }
        }

    if (queue_family->queueFlags & VK_QUEUE_TRANSFER_BIT) 
        { 
            queue_name += "{ Transfer } "; 

            if (queues.indices.graphics_family.value() != i) 
                {
                    queues.indices.transfer_family = i;
                    queues.priorities.push_back(std::vector<float>(queue_family->queueCount, 1.0f));
                    report(LOGGER::VLINE, "\t\tTransfer Family Set.");
                }
        }

    if (queue_family->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) 
        { queue_name += "{ Sparse Binding } "; }

    if (queue_name.empty()) 
        { queue_name = "~ Unknown ~"; }


    report(LOGGER::VLINE, "\t\t\tQueue Count: %d", queue_family->queueCount);
    report(LOGGER::VLINE, "\t\t\t %s", queue_name.c_str());
}

void GFXEngine::getQueueFamilies(VkPhysicalDevice scanned_device) 
    {
        report(LOGGER::VLINE, "\t .. Acquiring Queue Families ..");
        uint32_t _queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, nullptr);
        queues.families.resize(_queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(scanned_device, &_queue_family_count, queues.families.data());

        for (int i = 0; i < queues.families.size(); i++) 
            {
                report(LOGGER::VLINE, "\t\tQueue Family %d", i);

                // If we haven't found a present family yet, we'll take the first one we find
                if (queues.indices.present_family.value() == -1){
                        VkBool32 _present_support = false;  
                        vkGetPhysicalDeviceSurfaceSupportKHR(scanned_device, i, surface, &_present_support);

                        if (_present_support) 
                            { 
                                queues.indices.present_family = i; 
                                report(LOGGER::VLINE, "\t\tPresent Family Set.");    
                            }
                    }

                setQueueFamilyProperties(i);
            }

        // Check if the queues are complete and set the transfer family to the graphics family if not set
        if (!queues.indices.isComplete()) 
            { 
                report(LOGGER::VLINE, "\t\tQueue Families Incomplete. Setting Transfer Family to Graphics Family.");
                queues.indices.transfer_family = queues.indices.graphics_family.value();
                queues.indices.compute_family = queues.indices.graphics_family.value();
            }
    }

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

    ////////////////////////
    //  DEVICE PROVISION  //
    ////////////////////////

bool GFXEngine::deviceProvisioned(VkPhysicalDevice scanned_device)
    {
        getQueueFamilies(scanned_device);
        bool extensions_supported = checkDeviceExtensionSupport(scanned_device);

        bool swap_chain_adequate = false;
        if (extensions_supported) 
            {
                SwapChainSupportDetails swap_chain_support = querySwapChainSupport(scanned_device);
                swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
            }

        return queues.indices.isComplete() && extensions_supported && swap_chain_adequate;
    }


    //////////////////////////
    // PHYSICAL DEVICE INFO //
    //////////////////////////

void GFXEngine::createPhysicalDevice() 
    {
        report(LOGGER::VLINE, "\t .. Scanning for Physical Devices ..");

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
                VkPhysicalDeviceProperties device_properties;
                vkGetPhysicalDeviceProperties(device, &device_properties);
                report(LOGGER::VLINE, "\tScanning Device: %p - %s", device, device_properties.deviceName);
                if (device == VK_NULL_HANDLE) 
                    { continue; }

                if (deviceProvisioned(device))
                    { 
                        report(LOGGER::VLINE, "\tUsing Device: %s", device_properties.deviceName);

  
                        physical_device = device;
                        break; 
                    }
            }


        if (physical_device == VK_NULL_HANDLE) 
            { report(LOGGER::ERROR, "Vulkan: Failed to find a suitable GPU"); }

        return;
    }


    /////////////////////////
    // LOGICAL DEVICE INFO //
    /////////////////////////

VkDeviceQueueCreateInfo GFXEngine::getQueueCreateInfo(uint32_t queue_family)
    {
        return {
                sType: VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                queueFamilyIndex: queue_family,
                queueCount: queues.families[queue_family].queueCount,
                pQueuePriorities: queues.priorities[queue_family].data()
            };
    }

void GFXEngine::createLogicalDevice()
    {
        report(LOGGER::VLINE, "\t .. Creating Logical Device ..");
        VkPhysicalDeviceFeatures _device_features = {};

        std::vector<VkDeviceQueueCreateInfo> _queue_create_infos;
        std::set<uint32_t> _unique_queue_families = {
                queues.indices.graphics_family.value(), 
                queues.indices.present_family.value(),
                queues.indices.compute_family.value(),
                queues.indices.transfer_family.value()
            };

        report(LOGGER::VLINE, "\t .. Creating Queue Family ..");
        for (uint32_t _queue_family : _unique_queue_families) 
            {
                report(LOGGER::VLINE, "\t\tQueue Family: %d", _queue_family);
                report(LOGGER::VLINE, "\t\t\tQueue Count: %d", queues.families[_queue_family].queueCount);

                if (_queue_family == -1) { continue; }

                VkDeviceQueueCreateInfo _queue_info = getQueueCreateInfo(_queue_family);

                _queue_create_infos.push_back(_queue_info);
            }

        VkDeviceCreateInfo create_info = {
                sType: VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                queueCreateInfoCount: static_cast<uint32_t>(_queue_create_infos.size()),
                pQueueCreateInfos: _queue_create_infos.data(),
                pEnabledFeatures: &_device_features,
            };

        create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

        VK_TRY(vkCreateDevice(physical_device, &create_info, nullptr, &logical_device));

        vkGetDeviceQueue(logical_device, queues.indices.graphics_family.value(), 0, &queues.graphics);
        vkGetDeviceQueue(logical_device, queues.indices.present_family.value(), 0, &queues.present);
        vkGetDeviceQueue(logical_device, queues.indices.compute_family.value(), 0, &queues.compute);
        vkGetDeviceQueue(logical_device, queues.indices.transfer_family.value(), 0, &queues.transfer);

        //log();
    }