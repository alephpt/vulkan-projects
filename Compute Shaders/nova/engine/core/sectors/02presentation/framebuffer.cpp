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
                std::array<VkImageView, 1> _attachments =
                    { 
                        //color.view,
                        //depth.view,
                        swapchain.image_views[i] 
                    };

                VkFramebufferCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = static_cast<uint32_t>(_attachments.size()),
                    .pAttachments = _attachments.data(),
                    .width = swapchain.details.extent.width,
                    .height = swapchain.details.extent.height,
                    .layers = 1
                };

                VK_TRY(vkCreateFramebuffer(logical_device, &_create_info, nullptr, &swapchain.framebuffers[i]));
            }

        return;
    }
