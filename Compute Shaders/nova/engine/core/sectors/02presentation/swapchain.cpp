#include "../../core.h"
#include <set> 

    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails NovaCore::querySwapChainSupport(VkPhysicalDevice device)
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Support ..");
        
        SwapChainSupportDetails details = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t _format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, nullptr);

        if (_format_count != 0) 
            {
                details.formats.resize(_format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, details.formats.data());
            }

        uint32_t _present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, nullptr);

        if (_present_mode_count != 0) 
            {
                details.present_modes.resize(_present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, details.present_modes.data());
            }


        return details;
    }

static void selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats, VkSurfaceFormatKHR* surface_format)
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Surface Format ..");

        for (const auto& available_format : available_formats) 
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
                    { *surface_format = available_format; }
            }

        *surface_format = available_formats.front();

        return;
    }

static void selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes, VkPresentModeKHR* present_mode)
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Present Mode ..");

        for (const auto& available_present_mode : available_present_modes) 
            {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
                    { *present_mode = available_present_mode; }
            }

        *present_mode = VK_PRESENT_MODE_FIFO_KHR;

        return;
    }

static inline void selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D* extent)
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Extent ..");

        report(LOGGER::VERBOSE, "\t\tSurface Capabilities: Current Width: %d Current Height: %d", capabilities.currentExtent.width, capabilities.currentExtent.height);
        report(LOGGER::VERBOSE, "\t\tSurface Capabilities: Min Width: %d Min Height: %d", capabilities.minImageExtent.width, capabilities.minImageExtent.height);
        report(LOGGER::VERBOSE, "\t\tSurface Capabilities: Max Width: %d Max Height: %d", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
        report(LOGGER::VERBOSE, "\t\tSwap Extent: Width: %d Height: %d", extent->width, extent->height);


        if (capabilities.currentExtent.width != UINT32_MAX) 
            { 
                extent->width = capabilities.currentExtent.width;
                extent->height = capabilities.currentExtent.height;
            }

        return;
    }


void NovaCore::querySwapChainDetails()
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Details ..");

        if (swapchain.support.formats.empty() || swapchain.support.present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: SwapChain support is not available."); }

        selectSwapSurfaceFormat(swapchain.support.formats, &swapchain.details.surface);
        selectSwapPresentMode(swapchain.support.present_modes, &swapchain.details.present_mode);
        selectSwapExtent(swapchain.support.capabilities, &swapchain.details.extent);

        return;
    }

void NovaCore::createSwapchainInfoKHR(VkSwapchainCreateInfoKHR* create_info, uint32_t image_count) 
    {
        *create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = image_count,
            .imageFormat = swapchain.details.surface.format,
            .imageColorSpace = swapchain.details.surface.colorSpace,
            .imageExtent = swapchain.details.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = swapchain.support.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = swapchain.details.present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };
    }

// TODO: Wrap this in a class where we can just delete and recreate the swapchain using a singleton wrapper
void NovaCore::constructSwapChain() 
    {
        report(LOGGER::VLINE, "\t .. Constructing SwapChain ..");

        swapchain.support = querySwapChainSupport(physical_device);

        querySwapChainDetails();

        uint32_t _image_count = swapchain.support.capabilities.minImageCount + 1;

        if (swapchain.support.capabilities.maxImageCount > 0 && _image_count > swapchain.support.capabilities.maxImageCount) 
            { _image_count = swapchain.support.capabilities.maxImageCount; }

        //log();
        VkSwapchainCreateInfoKHR _create_info = {}; // TODO: This could be 1 line
        createSwapchainInfoKHR(&_create_info, _image_count);

        std::set<uint32_t> _unique_queue_families = { queues.indices.graphics_family.value(), queues.indices.present_family.value(), queues.indices.transfer_family.value(), queues.indices.compute_family.value() };
        std::vector<uint32_t> _queue_families(_unique_queue_families.begin(), _unique_queue_families.end());

        if (_queue_families.size() > 1) 
            {
                report(LOGGER::VLINE, "\t .. Multiple Queue Families Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                _create_info.queueFamilyIndexCount = static_cast<uint32_t>(_unique_queue_families.size());
                _create_info.pQueueFamilyIndices = _queue_families.data();
            }
        else
            {
                report(LOGGER::VLINE, "\t .. Single Queue Family Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                _create_info.queueFamilyIndexCount = 0;
                _create_info.pQueueFamilyIndices =  nullptr;
            }

        VkSwapchainPresentScalingCreateInfoEXT _scalability_info = {};
        if (framebuffer_resized) 
            {
                report(LOGGER::VERBOSE, "\t .. Framebuffer Resized ..");
                _scalability_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT;
                _scalability_info.scalingBehavior = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
                _create_info.pNext = &_scalability_info;
            }

        VK_TRY(vkCreateSwapchainKHR(logical_device, &_create_info, nullptr, &swapchain.instance));

        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, nullptr);
        swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, swapchain.images.data());

        report(LOGGER::VLINE, "\t .. SwapChain Constructed ..");

        return;
    }

void NovaCore::recreateSwapChain() 
    {
        report(LOGGER::VERBOSE, "Presentation - Recreating Swapchain ..");

        vkDeviceWaitIdle(logical_device);
        destroySwapChain();

        // TODO: Try chasing this down without the validation layer bugs being blocked 
        // to see if OUTOFDATE_KHR is the issue or if it is SUBOPTIMAL_KHR
        if (framebuffer_resized) {
            swapchain.support = querySwapChainSupport(physical_device);
            querySwapChainDetails();
        }

        constructSwapChain();
        constructImageViews();
        createColorResources();
        createDepthResources();
        createFrameBuffers();
        return;
    }

