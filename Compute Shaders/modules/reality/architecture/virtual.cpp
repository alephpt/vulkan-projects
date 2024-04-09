#include "../architect.h"
#include <set>

    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails Architect::querySwapChainSupport(VkPhysicalDevice device)
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


void Architect::querySwapChainDetails()
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Details ..");

        if (swapchain.support.formats.empty() || swapchain.support.present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: SwapChain support is not available."); }

        selectSwapSurfaceFormat(swapchain.support.formats, &swapchain.details.surface_format);
        selectSwapPresentMode(swapchain.support.present_modes, &swapchain.details.present_mode);
        selectSwapExtent(swapchain.support.capabilities, &swapchain.details.extent);

        return;
    }

void Architect::createSwapchainInfoKHR(VkSwapchainCreateInfoKHR* create_info, uint32_t image_count) 
    {
    *create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = swapchain.details.surface_format.format,
        .imageColorSpace = swapchain.details.surface_format.colorSpace,
        .imageExtent = swapchain.extent,
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

void Architect::constructSwapChain() 
    {
        report(LOGGER::VLINE, "\t .. Constructing SwapChain ..");

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

        VK_TRY(vkCreateSwapchainKHR(logical_device, &_create_info, nullptr, &swapchain.instance));

        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, nullptr);
        swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, swapchain.images.data());

        swapchain.format = swapchain.details.surface_format.format;
        swapchain.extent = swapchain.details.extent;

        report(LOGGER::VLINE, "\t .. SwapChain Constructed ..");

        return;
    }

VkImageViewCreateInfo Architect::createImageViewInfo(size_t image) {
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


void Architect::constructImageViews()
    {
        report(LOGGER::VLINE, "\t .. Constructing Image Views ..");

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

void Architect::createFrameBuffers()
    {
        report(LOGGER::VLINE, "Architect - Creating Frame Buffers ..");

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

void Architect::destroySwapChain() 
    {
        report(LOGGER::VERBOSE, "Architect - Destroying Swapchain ..");

        for (const auto& _frame_buffers : swapchain.framebuffers) 
            { vkDestroyFramebuffer(logical_device, _frame_buffers, nullptr); }
        
        swapchain.framebuffers.clear();

        
        for (const auto& _image_view : swapchain.image_views) 
            { vkDestroyImageView(logical_device, _image_view, nullptr); }

        swapchain.image_views.clear();

        vkDestroySwapchainKHR(logical_device, swapchain.instance, nullptr);

        return;
    }

void Architect::recreateSwapChain() 
    {
        report(LOGGER::VERBOSE, "Architect - Recreating Swapchain ..");

        vkDeviceWaitIdle(logical_device);
        destroySwapChain();

        constructSwapChain();
        constructImageViews();
        createFrameBuffers();

        return;
    }

void Architect::destroyVertexContext() 
    {
        report(LOGGER::VERBOSE, "Architect - Destroying Vertex Context ..");

        vkDestroyBuffer(logical_device, vertex.buffer, nullptr);
        vkFreeMemory(logical_device, vertex.memory, nullptr);

        return;
    }
