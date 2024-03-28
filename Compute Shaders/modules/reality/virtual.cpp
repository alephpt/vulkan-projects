#include "./virtual.h"


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
    {
        report(LOGGER::DLINE, "\t.. Querying SwapChain Support ..");
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details._capabilities);

        uint32_t _format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, nullptr);

        if (_format_count != 0) 
            {
                details._formats.resize(_format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, details._formats.data());
            }

        uint32_t _present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, nullptr);

        if (_present_mode_count != 0) 
            {
                details._present_modes.resize(_present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, details._present_modes.data());
            }

        return details;
    }

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
        report(LOGGER::ILINE, "\t.. Querying SwapChain Details ..");
        SwapChainDetails details;

        if (swap_chain_support._formats.empty() || swap_chain_support._present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: Swap chain support not available"); }

        details._surface_format = selectSwapSurfaceFormat(swap_chain_support._formats);
        details._present_mode = selectSwapPresentMode(swap_chain_support._present_modes);
        details._extent = selectSwapExtent(swap_chain_support._capabilities, window_extent);

        return details;
    }