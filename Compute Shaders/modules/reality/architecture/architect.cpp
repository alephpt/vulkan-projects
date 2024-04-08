#include "../architect.h"

#include <set>
#include <string>


    // Get Current Frame
FrameData& Architect::current_frame() { { return frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }

    ////////////////////////
    //  INSTANCE CREATION //
    ////////////////////////

Architect::Architect() 
    {
        _blankContext();
    }

Architect::~Architect() 
    {
        report(LOGGER::INFO, "Architect - Destroying Context ..");
        queues.deletion.flush();

        destroySwapChain();

        report(LOGGER::DLINE, "\t .. Destroying Semaphores, Fences and Command Pools ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(logical_device, frames[i].image_available, nullptr);
                vkDestroySemaphore(logical_device, frames[i].render_finished, nullptr);
                vkDestroyFence(logical_device, frames[i].in_flight, nullptr);
                vkDestroyCommandPool(logical_device, frames[i].command_pool, nullptr);
            }

        destroyGateway();

        report(LOGGER::DLINE, "\t .. Destroying Gateway and Render Pass ..");
        vkDestroyRenderPass(logical_device, render_pass, nullptr);

        report(LOGGER::DLINE, "\t .. Destroying Logical Device ..");
        vkDestroyDevice(logical_device, nullptr);

        report(LOGGER::DLINE, "\t .. Destroying Surface ..");
        vkDestroySurfaceKHR(instance, surface, nullptr);

        report(LOGGER::DLINE, "\t .. Destroying Instance ..");
        vkDestroyInstance(instance, nullptr);

        _blankContext();
        
    }


    ////////////////////////
    //  CONTEXT CREATION  //
    ////////////////////////

static Queues initQueues() 
    {
        return {
                .graphics = VK_NULL_HANDLE,
                .present = VK_NULL_HANDLE,
                .compute = VK_NULL_HANDLE,
                .families = {},
                .indices = {},
                .priorities = {}
            };
    }

static SwapChainSupportDetails initSwapChainSupport() 
    {
        return {
                .capabilities = {},
                .formats = {},
                .present_modes = {}
            };
    }

static SwapChainDetails initSwapChainDetails() 
    {
        return {
                .surface_format = {},
                .present_mode = {},
                .extent = {}
            };
    }

static SwapChainContext initSwapchain() {
    return {
            .instance = VK_NULL_HANDLE,
            .images = {},
            .image_views = {},
            .framebuffers = {},
            .format = VK_FORMAT_UNDEFINED,
            .extent = {0, 0},
            .support = initSwapChainSupport(),
            .details = initSwapChainDetails()
        };
}


void Architect::_blankContext() 
    {
        report(LOGGER::INFO, "Architect - No Context ..");

        instance = VK_NULL_HANDLE;
        physical_device = VK_NULL_HANDLE;
        logical_device = VK_NULL_HANDLE;
        surface = VK_NULL_HANDLE;
        render_pass = VK_NULL_HANDLE;
        queues = initQueues();
        swapchain = initSwapchain();
    }

// This should not be done like this
void Architect::setWindowExtent(VkExtent2D extent) 
    {
        swapchain.extent = extent;
        swapchain.details.extent = extent;
    }


    /////////////
    // LOGGING //
    /////////////

void Architect::logQueues() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Queues ..");
        report(LOGGER::DLINE, "\t\tGraphics: %p", queues.graphics);
        report(LOGGER::DLINE, "\t\tPresent: %p", queues.present);
        report(LOGGER::DLINE, "\t\tCompute: %p", queues.compute);
        report(LOGGER::DLINE, "\t\tFamilies: %d", queues.families.size());
        report(LOGGER::DLINE, "\t\tPriorities: %d", queues.priorities.size());
        report(LOGGER::DLINE, "\t\tPresent Family Index: %d", queues.indices.present_family.value());
        report(LOGGER::DLINE, "\t\tGraphics Family Index: %d", queues.indices.graphics_family.value());
        report(LOGGER::DLINE, "\t\tCompute Family Index: %d", queues.indices.compute_family.value());
    }

void Architect::logFrameData()
    {
        report(LOGGER::DEBUG, "\t .. Logging Frame Data ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                report(LOGGER::DLINE, "\t\tFrame %d", i);
                report(LOGGER::DLINE, "\t\t\tCommand Pool: %p", frames[i].command_pool);
                report(LOGGER::DLINE, "\t\t\tCommand Buffer: %p", frames[i].command_buffer);
                report(LOGGER::DLINE, "\t\t\tImage Available: %p", frames[i].image_available);
                report(LOGGER::DLINE, "\t\t\tRender Finished: %p", frames[i].render_finished);
                report(LOGGER::DLINE, "\t\t\tIn Flight: %p", frames[i].in_flight);
            }
    
    }

void Architect::logSwapChain() 
    {
        report(LOGGER::DEBUG, "\t .. Logging SwapChain ..");
        report(LOGGER::DLINE, "\t\tSwapchain: %p", swapchain.instance);
        report(LOGGER::DLINE, "\t\tImage Count: %d", swapchain.images.size());
        report(LOGGER::DLINE, "\t\tImage Views: %d", swapchain.image_views.size());
        report(LOGGER::DLINE, "\t\tFramebuffers: %d", swapchain.framebuffers.size());
        report(LOGGER::DLINE, "\t\tImage Format: %d", swapchain.format);
        report(LOGGER::DLINE, "\t\tSupport: %d Formats", swapchain.support.formats.size());
        report(LOGGER::DLINE, "\t\tSupport: %d Present Modes", swapchain.support.present_modes.size());
        report(LOGGER::DLINE, "\t\tDetails: %d Formats", swapchain.details.surface_format.format);
        report(LOGGER::DLINE, "\t\tDetails: %d Present Modes", swapchain.details.present_mode);
        report(LOGGER::DLINE, "\t\tExtent: %d x %d", swapchain.extent.width, swapchain.extent.height);
    }

void Architect::log() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Context ..");
        report(LOGGER::DLINE, "\t\tInstance: %p", instance);
        report(LOGGER::DLINE, "\t\tPhysical Device: %p", physical_device);
        report(LOGGER::DLINE, "\t\tLogical Device: %p", logical_device);
        report(LOGGER::DLINE, "\t\tSurface: %p", surface);
        logQueues();
        logSwapChain();
        report(LOGGER::DLINE, "\t\tRender Pass: %p", render_pass);
        report(LOGGER::DLINE, "\t\tPresent Info: %p", &present);
        logFrameData();
    }


    ////////////////////
    //  Device Queues //
    ////////////////////

void Architect::setQueueFamilyProperties(unsigned int i) {
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
        { queue_name += "{ Transfer } "; }

    if (queue_family->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) 
        { queue_name += "{ Sparse Binding } "; }

    if (queue_name.empty()) 
        { queue_name = "~ Unknown ~"; }

    report(LOGGER::VLINE, "\t\t\tQueue Count: %d", queue_family->queueCount);
    report(LOGGER::VLINE, "\t\t\t %s", queue_name.c_str());
}

void Architect::getQueueFamilies(VkPhysicalDevice scanned_device) 
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

bool Architect::deviceProvisioned(VkPhysicalDevice scanned_device)
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

void Architect::createPhysicalDevice() 
    {
        report(LOGGER::DLINE, "\t .. Scanning for Physical Devices ..");

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

VkDeviceQueueCreateInfo Architect::getQueueCreateInfo(uint32_t queue_family)
    {
        return {
                sType: VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                queueFamilyIndex: queue_family,
                queueCount: queues.families[queue_family].queueCount,
                pQueuePriorities: queues.priorities[queue_family].data()
            };
    }

void Architect::createLogicalDevice()
    {
        report(LOGGER::DLINE, "\t .. Creating Logical Device ..");
        VkPhysicalDeviceFeatures _device_features = {};

        std::vector<VkDeviceQueueCreateInfo> _queue_create_infos;
        std::set<uint32_t> _unique_queue_families = {
                queues.indices.graphics_family.value(), 
                queues.indices.present_family.value(),
                queues.indices.compute_family.value()
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

        //log();
    }