#include "./virtual.h"

static VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) 
    {
        report(LOGGER::DLINE, "\t.. Selecting Swap Surface Format ..");
        for (const auto& available_format : available_formats) 
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
                    { return available_format; }
            }

        return available_formats[0];
    }


static VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) 
    {
        report(LOGGER::DLINE, "\t.. Selecting Swap Present Mode ..");
        for (const auto& available_present_mode : available_present_modes) 
            {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
                    { return available_present_mode; }
            }

        return VK_PRESENT_MODE_FIFO_KHR;
    }


static VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D window_extent) 
    {
        report(LOGGER::DLINE, "\t.. Selecting Swap Extent ..");
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
        report(LOGGER::DLINE, "\t.. Querying SwapChain Details ..");
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
        report(LOGGER::DLINE, "\t.. Constructing SwapChain ..");
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

        std::vector<VkQueueFamilyProperties> _queue_families = getQueueFamilies(context->physical_device); // THIS NEEDS TO BE CALLED ONCE WHEN WE CREATE THE PHYSICAL DEVICE
        QueueFamilyIndices _indices = findQueueFamilies(context->physical_device, context->surface, _queue_families);
        uint32_t _queue_family_indices[] = { _indices.graphics_family.value(), _indices.present_family.value() };

        if (_indices.graphics_family != _indices.present_family)
            {
                report(LOGGER::DLINE, "\t.. Concurrent Queue Families Achieved ..");
                _create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                _create_info.queueFamilyIndexCount = 1;
                _create_info.pQueueFamilyIndices = _queue_family_indices;
            }
        else
            {
                report(LOGGER::ERROR, "\t.. Single Queue Family Achieved ..");
                _create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                _create_info.queueFamilyIndexCount = 0;
                _create_info.pQueueFamilyIndices = nullptr;
            }

        VK_TRY(vkCreateSwapchainKHR(context->logical_device, &_create_info, nullptr, &context->swapchain.instance));

        vkGetSwapchainImagesKHR(context->logical_device, context->swapchain.instance, &_image_count, nullptr);
        context->swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(context->logical_device, context->swapchain.instance, &_image_count, context->swapchain.images.data());

        context->swapchain.format = swap_chain_details.surface_format.format;
        context->swapchain.extent = swap_chain_details.extent;

        report(LOGGER::INFO, "\t.. SwapChain Constructed ..");
        return; 
    }