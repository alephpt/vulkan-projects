#include "../../core.h"



#define STB_IMAGE_IMPLEMENTATION
#include "../../components/extern/stb_image.h"
const char* TEXTURE_PATH = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/nova/engine/core/components/extern/texture.png";

    /////////////////////////////
    // TEXTURE BUFFER CREATION //
    /////////////////////////////


static inline VkImageCreateInfo _createImageInfo(uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t mip_lvls = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = { .width = w, .height = h, .depth = 1 },
            .mipLevels = mip_lvls,
            .arrayLayers = 1,
            .samples = samples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = _IMAGE_LAYOUT_UNDEFINED,
        };
    }

inline void NovaCore::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, VkQueue& queue, VkCommandPool& pool, uint32_t mips = 1)
    {
        report(LOGGER::VLINE, "\t .. Transitioning Image Layout ..");
        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(pool);
        VkImageMemoryBarrier _barrier = getMemoryBarrier(image, old_layout, new_layout, mips);
        VkPipelineStageFlags _src_stage, _dst_stage;

        if (old_layout == _IMAGE_LAYOUT_UNDEFINED && new_layout == _IMAGE_LAYOUT_DST) 
            {
                _barrier.srcAccessMask = 0;
                _barrier.dstAccessMask = _TRANSFER_WRITE_BIT;

                _src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                _dst_stage = _PIPELINE_TRANSFER_BIT;
            } 
        else if (old_layout == _IMAGE_LAYOUT_DST && new_layout == _IMAGE_LAYOUT_READ_ONLY) 
            {
                _barrier.srcAccessMask = _TRANSFER_WRITE_BIT;
                _barrier.dstAccessMask = _SHADER_READ_BIT;

                _src_stage = _PIPELINE_TRANSFER_BIT;
                _dst_stage = _PIPELINE_FRAGMENT_BIT;
            } 
        else 
            { report(LOGGER::ERROR, "Scene - Unsupported Layout Transition .."); return; }

        vkCmdPipelineBarrier(_ephemeral_cmd, _src_stage, _dst_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

        char _msg[] = "Transition Image Layout";
        flushCommandBuffer(_ephemeral_cmd, _msg, queue, pool);
    }

static inline VkBufferImageCopy _getImageCopyRegion(uint32_t width, uint32_t height) 
    {
        return {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = { .aspectMask = _IMAGE_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
            .imageOffset = { .x = 0, .y = 0, .z = 0 },
            .imageExtent = { .width = width, .height = height, .depth = 1 }
        };
    }

inline void NovaCore::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, VkQueue& queue, VkCommandPool& pool)
    {
        report(LOGGER::VLINE, "\t .. Copying Buffer to Image ..");

        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(pool);

        VkBufferImageCopy _region = _getImageCopyRegion(width, height);
        vkCmdCopyBufferToImage(_ephemeral_cmd, buffer, image, _IMAGE_LAYOUT_DST, 1, &_region);

        char _msg[] = "Copy Buffer";
        flushCommandBuffer(_ephemeral_cmd, _msg, queue, pool);

        return;
    }

static inline void destroyImage(VkDevice& device, const VkImage& image, const VkDeviceMemory& memory) 
    {
        if (image != VK_NULL_HANDLE) {
            vkDestroyImage(device, image, nullptr);
            report(LOGGER::VLINE, "\t .. Destroying Image ..");
        }

        if (memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, memory, nullptr);
            report(LOGGER::VLINE, "\t .. Freeing Memory ..");
        }
    }

void NovaCore::createImage(uint32_t w, uint32_t h, uint32_t mips, VkSampleCountFlagBits samples, 
                        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags props,
                        VkImage& image, VkDeviceMemory& memory)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Image ..");

        VkImageCreateInfo _image_info = _createImageInfo(w, h, format, tiling, usage, mips, samples);
        VK_TRY(vkCreateImage(logical_device, &_image_info, nullptr, &image));

        VkMemoryRequirements _mem_reqs;
        vkGetImageMemoryRequirements(logical_device, image, &_mem_reqs);

        VkMemoryAllocateInfo _alloc_info = getMemoryAllocateInfo(_mem_reqs, props);
        VK_TRY(vkAllocateMemory(logical_device, &_alloc_info, nullptr, &memory));
        VK_TRY(vkBindImageMemory(logical_device, image, memory, 0));

        queues.deletion.push_fn([=]() { destroyImage(logical_device, image, memory); });

        return;
    }

void NovaCore::createTextureImage() 
    {
        report(LOGGER::VLINE, "\t .. Creating Texture Buffer ..");

        // Load the texture image
        int _tex_width, _tex_height, _tex_channels;
        stbi_uc* _pixels = stbi_load(TEXTURE_PATH, &_tex_width, &_tex_height, &_tex_channels, STBI_rgb_alpha);
        VkDeviceSize _image_size = _tex_width * _tex_height * 4;
        mip_lvls = static_cast<uint32_t>(std::floor(std::log2(std::max(_tex_width, _tex_height)))) + 1;

        if (!_pixels) 
            { report(LOGGER::ERROR, "Scene - Failed to load texture image .."); return; }

        // Create a staging buffer to copy the image data to
        BufferContext _staging;
        createBuffer(_image_size, _TRANSFER_SRC_BIT, _STAGING_PROPERTIES_BIT, &_staging);

        // Copy the image data to the staging buffer
        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _image_size, 0, &data);
        memcpy(data, _pixels, static_cast<size_t>(_image_size));
        vkUnmapMemory(logical_device, _staging.memory);

        stbi_image_free(_pixels);

        createImage(_tex_width, _tex_height, mip_lvls, VK_SAMPLE_COUNT_1_BIT, _SRGB_FORMAT_888, VK_IMAGE_TILING_OPTIMAL, _IMAGE_TRANSFER_BIT, _LOCAL_DEVICE_BIT, texture.image, texture.memory);

        // Can we do this on transfer?
        // Transition the image to a layout that is optimal for copying data to
        transitionImageLayout(texture.image, _SRGB_FORMAT_888, _IMAGE_LAYOUT_UNDEFINED, _IMAGE_LAYOUT_DST, queues.graphics, queues.command_pool, mip_lvls); 
        copyBufferToImage(_staging.buffer, texture.image, static_cast<uint32_t>(_tex_width), static_cast<uint32_t>(_tex_height), queues.graphics, queues.command_pool);

        // We need to trigger the texture image to be deleted before the pipeline goes out of scope
        queues.deletion.push_fn([=]() { destroyImage(logical_device, texture.image, texture.memory); } );

        // Clean up the staging buffer
        destroyBuffer(&_staging);

        generateMipmaps(texture.image, _SRGB_FORMAT_888, _tex_width, _tex_height, mip_lvls);

        return;
    }

    /////////////////////////////////
    // TEXTURE IMAGE VIEW CREATION //
    /////////////////////////////////

// TODO: Look into other view types
static inline VkImageViewCreateInfo _getImageViewInfo(VkImage& image, VkFormat format) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = { 
                .aspectMask = _IMAGE_COLOR_BIT, 
                .baseMipLevel = 0, 
                .levelCount = 1, 
                .baseArrayLayer = 0, 
                .layerCount = 1 
            }
        };
    }

// TODO: Allow this to pass in dynamic image views
void NovaCore::createTextureImageView() 
    {
        report(LOGGER::VLINE, "\t .. Creating Texture Image View ..");

        VkImageViewCreateInfo _view_info = _getImageViewInfo(texture.image, _SRGB_FORMAT_888);
        VK_TRY(vkCreateImageView(logical_device, &_view_info, nullptr, &texture.view));
        queues.deletion.push_fn([=]() { vkDestroyImageView(logical_device, texture.view, nullptr); });
    }



    //////////////////////////////
    // TEXTURE SAMPLER CREATION //
    //////////////////////////////

static inline VkSamplerCreateInfo _getSamplerInfo(VkPhysicalDeviceProperties& props, float mip_levels) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = props.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = mip_levels,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
    } 

void NovaCore::constructTextureSampler() {
    report(LOGGER::VLINE, "\t .. Creating Texture Sampler ..");

    VkPhysicalDeviceProperties _props;
    vkGetPhysicalDeviceProperties(physical_device, &_props);
    VkSamplerCreateInfo _sampler_info = _getSamplerInfo(_props, static_cast<float>(mip_lvls));

    VK_TRY(vkCreateSampler(logical_device, &_sampler_info, nullptr, &texture.sampler));

    queues.deletion.push_fn([=]() { 
        report(LOGGER::VLINE, "\t .. Destroying Texture Sampler ..");
        vkDestroySampler(logical_device, texture.sampler, nullptr); });
    queues.deletion.push_fn([=]() { 
        report(LOGGER::VLINE, "\t .. Destroying Texture Image View ..");
        vkDestroyImageView(logical_device, texture.view, nullptr); });
}



