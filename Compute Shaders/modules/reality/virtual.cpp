#include "./virtual.h"
#include "virtual.h"

static VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) 
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Surface Format ..");
        for (const auto& available_format : available_formats) 
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
                    { return available_format; }
            }

        return available_formats[0];
    }


static VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) 
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Present Mode ..");
        for (const auto& available_present_mode : available_present_modes) 
            {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
                    { return available_present_mode; }
            }

        return VK_PRESENT_MODE_FIFO_KHR;
    }


static VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D window_extent) 
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Extent ..");
        if (capabilities.currentExtent.width != UINT32_MAX) 
            { return capabilities.currentExtent; }
        else 
            {
                window_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, window_extent.width));
                window_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, window_extent.height));

                return window_extent;
            }
    }


SwapChainDetails querySwapChainDetails(SwapChainSupportDetails swap_chain_support, VkExtent2D window_extent)
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Details ..");
        SwapChainDetails details;

        if (swap_chain_support.formats.empty() || swap_chain_support.present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: Swap chain support not available"); }

        details.surface_format = selectSwapSurfaceFormat(swap_chain_support.formats);
        details.present_mode = selectSwapPresentMode(swap_chain_support.present_modes);
        details.extent = selectSwapExtent(swap_chain_support.capabilities, window_extent);

        return details;
    }


void constructSwapChain(SwapChainDetails swap_chain_details, SwapChainSupportDetails swap_chain_support, EngineContext *context) 
    {
        report(LOGGER::DLINE, "\t .. Constructing SwapChain ..");
        uint32_t _image_count = swap_chain_support.capabilities.minImageCount + 1;

        if (swap_chain_support.capabilities.maxImageCount > 0 && _image_count > swap_chain_support.capabilities.maxImageCount) 
            { _image_count = swap_chain_support.capabilities.maxImageCount; }

        VkSwapchainCreateInfoKHR _create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = context->surface,
            .minImageCount = _image_count,
            .imageFormat = swap_chain_details.surface_format.format,
            .imageColorSpace = swap_chain_details.surface_format.colorSpace,
            .imageExtent = swap_chain_details.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = swap_chain_support.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = swap_chain_details.present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        // Add Queue Family Sharing if Graphics and Present Queues are Different
        if (context->queues.indices.graphics_family != context->queues.indices.present_family)
            {
                report(LOGGER::VLINE, "\t .. Concurrent Queue Families Achieved ..");
                _create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                _create_info.queueFamilyIndexCount = context->queues.priorities.size();
                _create_info.pQueueFamilyIndices = &context->queues.indices.graphics_family.value();
            }
        else
            {
                report(LOGGER::VLINE, "\t .. Single Queue Family Achieved ..");
                _create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                _create_info.queueFamilyIndexCount = 0;
                _create_info.pQueueFamilyIndices =  nullptr;
            }

        VK_TRY(vkCreateSwapchainKHR(context->logical_device, &_create_info, nullptr, &context->swapchain.instance));

        vkGetSwapchainImagesKHR(context->logical_device, context->swapchain.instance, &_image_count, nullptr);
        context->swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(context->logical_device, context->swapchain.instance, &_image_count, context->swapchain.images.data());

        context->swapchain.format = swap_chain_details.surface_format.format;
        context->swapchain.extent = swap_chain_details.extent;

        report(LOGGER::VLINE, "\t .. SwapChain Constructed ..");
        return;
    }

    void constructImageViews(EngineContext *context)
    {
        report(LOGGER::DLINE, "\t .. Constructing Image Views ..");
        context->swapchain.image_views.resize(context->swapchain.images.size());

        for (size_t i = 0; i < context->swapchain.images.size(); i++) 
            {
                VkImageViewCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = context->swapchain.images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = context->swapchain.format,
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

                VK_TRY(vkCreateImageView(context->logical_device, &_create_info, nullptr, &context->swapchain.image_views[i]));
            }

        report(LOGGER::VLINE, "\t .. Image Views Constructed ..");
        return;
    }


    ///////////////////////////
    // FRAME BUFFER CREATION //
    ///////////////////////////

void createFrameBuffers(EngineContext *context)
    {
        report(LOGGER::DLINE, "\t .. Creating Frame Buffers ..");
        context->swapchain.framebuffers.resize(context->swapchain.image_views.size());

        for (size_t i = 0; i < context->swapchain.image_views.size(); i++) 
            {
                VkImageView _attachments[] = { context->swapchain.image_views[i] };

                VkFramebufferCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = context->render_pass,
                    .attachmentCount = 1,
                    .pAttachments = _attachments,
                    .width = context->swapchain.extent.width,
                    .height = context->swapchain.extent.height,
                    .layers = 1
                };

                VK_TRY(vkCreateFramebuffer(context->logical_device, &_create_info, nullptr, &context->swapchain.framebuffers[i]));
            }

        return;
    }