#include "../../core.h"

    ///////////////////////
    // MIPMAP GENERATION //
    ///////////////////////


VkImageMemoryBarrier NovaCore::getMemoryBarrier(VkImage& image, VkImageLayout& _old_layout, VkImageLayout& _new_layout, uint32_t mip_level)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = _TRANSFER_WRITE_BIT,
            .dstAccessMask = _TRANSFER_READ_BIT,
            .oldLayout = _old_layout,
            .newLayout = _new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = { 
                    .aspectMask = _IMAGE_COLOR_BIT, 
                    .baseMipLevel = 0, 
                    .levelCount = mip_level,
                    .baseArrayLayer = 0, 
                    .layerCount = 1 
                }
        };
    }

static inline void _setTransferBarrier(VkImageMemoryBarrier& barrier, uint32_t mip_level) 
    {
        barrier.subresourceRange.baseMipLevel = mip_level;
        barrier.oldLayout = _IMAGE_LAYOUT_DST;
        barrier.newLayout = _IMAGE_LAYOUT_SRC;
        barrier.srcAccessMask = _TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = _TRANSFER_READ_BIT;
    }

static inline VkImageBlit _getBlit(uint32_t mip_level, int32_t width, int32_t height) 
    {
        return {
            .srcSubresource = { .aspectMask = _IMAGE_COLOR_BIT, .mipLevel = mip_level - 1, .baseArrayLayer = 0, .layerCount = 1 },
            .srcOffsets = { {0, 0, 0}, {width, height, 1} },
            .dstSubresource = { .aspectMask = _IMAGE_COLOR_BIT, .mipLevel = mip_level, .baseArrayLayer = 0, .layerCount = 1 },
            .dstOffsets = { {0, 0, 0}, {width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1} }
        };
    }

static inline void _setReadBarrier(VkImageMemoryBarrier& barrier, uint32_t mip_level) 
    {
        barrier.subresourceRange.baseMipLevel = mip_level;
        barrier.oldLayout = _IMAGE_LAYOUT_SRC;
        barrier.newLayout = _IMAGE_LAYOUT_READ_ONLY;
        barrier.srcAccessMask = _TRANSFER_READ_BIT;
        barrier.dstAccessMask = _SHADER_READ_BIT;
    }

static inline void _setFinalBarrier(VkImageMemoryBarrier& barrier, uint32_t mip_level) 
    {
        barrier.subresourceRange.baseMipLevel = mip_level;
        barrier.oldLayout = _IMAGE_LAYOUT_DST;
        barrier.newLayout = _IMAGE_LAYOUT_READ_ONLY;
        barrier.srcAccessMask = _TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = _SHADER_READ_BIT;
    }

void NovaCore::generateMipmaps(VkImage& image, VkFormat format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels) 
    {
        report(LOGGER::VLINE, "\t .. Generating Mipmaps ..");

        VkFormatProperties _format_props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &_format_props);

        if (!(_format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
            { report(LOGGER::ERROR, "Scene - Texture Image Format does not support linear blitting .."); return; }

        VkCommandBuffer _ephemeral_command = createEphemeralCommand(queues.transfer.pool);

        VkImageLayout _old_layout = _IMAGE_LAYOUT_DST;
        VkImageLayout _new_layout = _IMAGE_LAYOUT_SRC;
        VkImageMemoryBarrier _barrier = getMemoryBarrier(image, _old_layout, _new_layout);

        int _mip_width = tex_width;
        int _mip_height = tex_height;

        for (uint32_t i = 1; i < mip_levels; i++) 
            {
                report(LOGGER::VLINE, "\t\t .. Mipmap Level: %d", i);
                _setTransferBarrier(_barrier, i - 1);   
                vkCmdPipelineBarrier(_ephemeral_command, _PIPELINE_TRANSFER_BIT, _PIPELINE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
                VkImageBlit _blit = _getBlit(i, _mip_width, _mip_height);
                vkCmdBlitImage(_ephemeral_command, image, _IMAGE_LAYOUT_SRC, image, _IMAGE_LAYOUT_DST, 1, &_blit, VK_FILTER_LINEAR);
                _setReadBarrier(_barrier, i);
                vkCmdPipelineBarrier(_ephemeral_command, _PIPELINE_TRANSFER_BIT, _PIPELINE_FRAGMENT_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

                if (_mip_width > 1) _mip_width /= 2;
                if (_mip_height > 1) _mip_height /= 2;
            }

        _setFinalBarrier(_barrier, mip_levels - 1);
        vkCmdPipelineBarrier(_ephemeral_command, _PIPELINE_TRANSFER_BIT, _PIPELINE_FRAGMENT_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

        char _msg[] = "Generate Mipmaps";
        flushCommandBuffer(_ephemeral_command, _msg, queues.graphics, queues.transfer.pool); 

        return;
    }
