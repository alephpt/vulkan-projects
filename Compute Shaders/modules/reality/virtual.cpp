#include "./architect.h"

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


static void selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D* window_extent) 
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Extent ..");

        if (capabilities.currentExtent.width != UINT32_MAX) 
            { 
                window_extent->width = capabilities.currentExtent.width;
                window_extent->height = capabilities.currentExtent.height;
            }
        else 
            {
                uint32_t width = window_extent->width;
                uint32_t height = window_extent->height;

                window_extent->width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
                window_extent->height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));
            }

        return;
    }


SwapChainDetails EngineContext::querySwapChainDetails()
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Details ..");

        SwapChainDetails details;

        if (swapchain.support.formats.empty() || swapchain.support.present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: SwapChain support is not available."); }

        selectSwapSurfaceFormat(swapchain.support.formats, &details.surface_format);
        selectSwapPresentMode(swapchain.support.present_modes, &details.present_mode);
        selectSwapExtent(swapchain.support.capabilities, &window_extent);

        return details;
    }

VkSwapchainCreateInfoKHR EngineContext::createSwapchainInfoKHR() {
    log();

    return {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = swapchain.support.capabilities.minImageCount + 1,
        .imageFormat = swapchain.details.surface_format.format,
        .imageColorSpace = swapchain.details.surface_format.colorSpace,
        .imageExtent = swapchain.details.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = swapchain.support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchain.details.present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
}

void EngineContext::constructSwapChain() 
    {
        report(LOGGER::DLINE, "\t .. Constructing SwapChain ..");

        uint32_t _image_count = swapchain.support.capabilities.minImageCount + 1;

        if (swapchain.support.capabilities.maxImageCount > 0 && _image_count > swapchain.support.capabilities.maxImageCount) 
            { _image_count = swapchain.support.capabilities.maxImageCount; }

        VkSwapchainCreateInfoKHR _create_info = createSwapchainInfoKHR();

        report(LOGGER::VLINE, "\t .. SwapChain Info Constructed ..");
        report(LOGGER::DEBUG, "\tCreate Info minImageCount: %d", _create_info.minImageCount);
        report(LOGGER::DEBUG, "\tCreate Info imageFormat: %d", _create_info.imageFormat);
        report(LOGGER::DEBUG, "\tCreate Info imageColorSpace: %d", _create_info.imageColorSpace);
        report(LOGGER::DEBUG, "\tCreate Info imageExtent: %d, %d", _create_info.imageExtent.width, _create_info.imageExtent.height);
        report(LOGGER::DEBUG, "\tCreate Info imageArrayLayers: %d", _create_info.imageArrayLayers);
        report(LOGGER::DEBUG, "\tCreate Info imageUsage: %d", _create_info.imageUsage);
        report(LOGGER::DEBUG, "\tCreate Info imageSharingMode: %d", _create_info.imageSharingMode);
        report(LOGGER::DEBUG, "\tCreate Info preTransform: %d", _create_info.preTransform);
        report(LOGGER::DEBUG, "\tCreate Info compositeAlpha: %d", _create_info.compositeAlpha);
        report(LOGGER::DEBUG, "\tCreate Info presentMode: %d", _create_info.presentMode);
        report(LOGGER::DEBUG, "\tCreate Info clipped: %d", _create_info.clipped);

        // Add Queue Family Sharing if Graphics and Present Queues are Different
        if (queues.indices.graphics_family != queues.indices.present_family)
            {
                report(LOGGER::VLINE, "\t .. Concurrent Queue Families Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                _create_info.queueFamilyIndexCount = queues.priorities.size();
                _create_info.pQueueFamilyIndices = &queues.indices.graphics_family.value();
            }
        else
            {
                report(LOGGER::VLINE, "\t .. Single Queue Family Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                _create_info.queueFamilyIndexCount = 0;
                _create_info.pQueueFamilyIndices =  nullptr;
            }

        VK_TRY(vkCreateSwapchainKHR(logical_device, &_create_info, nullptr, &swapchain.instance));

        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, nullptr);
        swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, swapchain.images.data());

        swapchain.format = swapchain.details.surface_format.format;
        swapchain.extent = swapchain.details.extent;

        report(LOGGER::VLINE, "\t .. SwapChain Constructed ..");

        return;
    }

VkImageViewCreateInfo EngineContext::createImageViewInfo(size_t image) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchain.images[image],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
}


void EngineContext::constructImageViews()
    {
        report(LOGGER::DLINE, "\t .. Constructing Image Views ..");

        swapchain.image_views.resize(swapchain.images.size());

        for (size_t i = 0; i < swapchain.images.size(); i++) 
            {
                VkImageViewCreateInfo _create_info = createImageViewInfo(i);
                VK_TRY(vkCreateImageView(logical_device, &_create_info, nullptr, &swapchain.image_views[i]));
            }

        report(LOGGER::VLINE, "\t .. Image Views Constructed ..");

        return;
    }


    ///////////////////////////
    // FRAME BUFFER CREATION //
    ///////////////////////////

void EngineContext::createFrameBuffers()
    {
        report(LOGGER::DLINE, "\t .. Creating Frame Buffers ..");

        swapchain.framebuffers.resize(swapchain.image_views.size());

        for (size_t i = 0; i < swapchain.image_views.size(); i++) 
            {
                VkImageView _attachments[] = { swapchain.image_views[i] };

                VkFramebufferCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = 1,
                    .pAttachments = _attachments,
                    .width = swapchain.extent.width,
                    .height = swapchain.extent.height,
                    .layers = 1
                };

                VK_TRY(vkCreateFramebuffer(logical_device, &_create_info, nullptr, &swapchain.framebuffers[i]));
            }

        return;
    }

void EngineContext::destroySwapChain() 
    {
        report(LOGGER::INFO, "Matrix - Destroying Swapchain ..");

        for (const auto _frame_buffers : swapchain.framebuffers) 
            { vkDestroyFramebuffer(logical_device, _frame_buffers, nullptr); }
        
        for (const auto _image_view : swapchain.image_views) 
            { vkDestroyImageView(logical_device, _image_view, nullptr); }

        vkDestroySwapchainKHR(logical_device, swapchain.instance, nullptr);

        return;
    }

void EngineContext::recreateSwapChain() 
    {
        report(LOGGER::INFO, "Matrix - Creating Swapchain ..");

        vkDeviceWaitIdle(logical_device);
        destroySwapChain();

        constructSwapChain();
        constructImageViews();
        createFrameBuffers();

        return;
    }


    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails EngineContext::querySwapChainSupport(VkPhysicalDevice device)
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

