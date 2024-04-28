#include "../../core.h"


VkImageView NovaCore::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect, uint32_t mip_levels)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Image View ..");

        VkImageViewCreateInfo _view_info = createImageViewInfo(image, format, aspect, 1);
        VkImageView view;
        VK_TRY(vkCreateImageView(logical_device, &_view_info, nullptr, &view));

        queues.deletion.push_fn([=]() { vkDestroyImageView(logical_device, view, nullptr); });

        return view;
    }

VkImageViewCreateInfo NovaCore::createImageViewInfo(VkImage img, VkFormat fmt, VkImageAspectFlags aspect, uint32_t mips)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = img,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = fmt,
            .subresourceRange = {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = mips,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
    }


void NovaCore::constructImageViews()
    {
        report(LOGGER::VLINE, "\t .. Constructing Image Views ..");

        swapchain.image_views.resize(swapchain.images.size());

        for (size_t i = 0; i < swapchain.images.size(); i++) 
            {
                VkImageViewCreateInfo _create_info = createImageViewInfo(swapchain.images[i], swapchain.details.surface.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
                VK_TRY(vkCreateImageView(logical_device, &_create_info, nullptr, &swapchain.image_views[i]));
            }

        report(LOGGER::VLINE, "\t .. Image Views Constructed ..");

        return;
    }