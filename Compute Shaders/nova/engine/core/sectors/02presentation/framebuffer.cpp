#include "../../core.h"

    ///////////////////////////
    // FRAME BUFFER CREATION //
    ///////////////////////////

void NovaCore::createFrameBuffers()
    {
        report(LOGGER::VLINE, "Presentation - Creating Frame Buffers ..");

        swapchain.framebuffers.resize(swapchain.image_views.size());

        for (size_t i = 0; i < swapchain.image_views.size(); i++) 
            {
                // std::array<VkImageView, 3> _attachments =
                //     { 
                //         color.view,
                //         depth.view,
                //         swapchain.image_views[i] 
                //     };
                VkImageView _attachments[] = { swapchain.image_views[i] };

                VkFramebufferCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = 1,
                    .pAttachments = _attachments,
                    .width = swapchain.details.extent.width,
                    .height = swapchain.details.extent.height,
                    .layers = 1
                };

                VK_TRY(vkCreateFramebuffer(logical_device, &_create_info, nullptr, &swapchain.framebuffers[i]));
            }

        return;
    }
