#include "../core.h"
#include <cstring>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../components/extern/stb_image.h"
const char* TEXTURE_PATH = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/nova/engine/core/components/extern/texture.png";

// Do we need to move this to the graphics pipeline?

    ////////////////////////////
    // OBJECT BUFFER CREATION //
    ////////////////////////////

static const VkBufferUsageFlags _TRANSFER_SRC_BIT = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
static const VkBufferUsageFlags _INDEX_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
static const VkBufferUsageFlags _VERTEX_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
static const VkBufferUsageFlags _IMAGE_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
static const VkBufferUsageFlags _IMAGE_TRANSFER_BIT = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
static const VkMemoryPropertyFlags _STAGING_PROPERTIES_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
static const VkMemoryPropertyFlags _LOCAL_DEVICE_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
static const VkImageLayout _IMAGE_LAYOUT_DST = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
static const VkImageLayout _IMAGE_LAYOUT_SRC = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; 
static const VkAccessFlags _TRANSFER_WRITE_BIT = VK_ACCESS_TRANSFER_WRITE_BIT;
static const VkAccessFlags _TRANSFER_READ_BIT = VK_ACCESS_TRANSFER_READ_BIT;
static const VkAccessFlags _SHADER_READ_BIT = VK_ACCESS_SHADER_READ_BIT;
static const VkImageLayout _IMAGE_LAYOUT_UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED;
static const VkImageLayout _IMAGE_LAYOUT_READ_ONLY = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
static const VkPipelineStageFlagBits _PIPELINE_TRANSFER_BIT = VK_PIPELINE_STAGE_TRANSFER_BIT;
static const VkFormat _SRGB_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
static const VkImageAspectFlagBits _IMAGE_COLOR_BIT = VK_IMAGE_ASPECT_COLOR_BIT;
static const VkImageUsageFlags _COLOR_ATTACHMENT_BIT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

void NovaCore::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(graphics_pipeline->vertices[0]) * graphics_pipeline->vertices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _TRANSFER_SRC_BIT, _STAGING_PROPERTIES_BIT, &_staging);

        // We do this to copy the data from the CPU to the GPU
        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data); // This maps the memory to the CPU
        memcpy(data, graphics_pipeline->vertices.data(), (size_t)_buffer_size);           // This copies the data to the memory
        vkUnmapMemory(logical_device, _staging.memory);                          // This unmaps the memory from the CPU

        // We create the buffer that will be used by the GPU
        createBuffer(_buffer_size, _VERTEX_BUFFER_BIT, _LOCAL_DEVICE_BIT, &vertex);
        copyBuffer(_staging.buffer, vertex.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void NovaCore::constructIndexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Index Buffer ..");

        VkDeviceSize _buffer_size = sizeof(graphics_pipeline->indices[0]) * graphics_pipeline->indices.size();
        report(LOGGER::VLINE, "\t\t .. Buffer Size: %d", _buffer_size);

        BufferContext _staging;
        createBuffer(_buffer_size, _TRANSFER_SRC_BIT, _STAGING_PROPERTIES_BIT, &_staging);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, graphics_pipeline->indices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _INDEX_BUFFER_BIT, _LOCAL_DEVICE_BIT, &index);
        copyBuffer(_staging.buffer, index.buffer, _buffer_size);
 
        destroyBuffer(&_staging);

        return;
    }

void NovaCore::destroyVertexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&vertex); return; }

void NovaCore::destroyIndexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&index); return; }

    /////////////////////////////
    // UNIFORM BUFFER CREATION //
    /////////////////////////////

static const VkBufferUsageFlags _uniform_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
static const VkMemoryPropertyFlags _uniform_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

void NovaCore::constructUniformBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Uniform Buffer ..");

        VkDeviceSize _buffer_size = sizeof(MVP);

        uniform.resize(MAX_FRAMES_IN_FLIGHT);
        uniform_data.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                createBuffer(_buffer_size, _uniform_usage, _uniform_properties, &uniform[i]);
                vkMapMemory(logical_device, uniform[i].memory, 0, _buffer_size, 0, &uniform_data[i]);
                queues.deletion.push_fn([=]() { vkUnmapMemory(logical_device, uniform[i].memory); });
            }
    }

void NovaCore::updateUniformBuffer(uint32_t current_frame)
    {
        static auto _s_t = std::chrono::high_resolution_clock::now();
        auto _c_t = std::chrono::high_resolution_clock::now();
        float _e_t = std::chrono::duration<float, std::chrono::seconds::period>(_c_t - _s_t).count();

        // Model Coordinate System controls the position of the object in the world
        // View Coordinate System controls the position of the camera in the world
        // Projection Coordinate System controls the perspective of the camera

        MVP _mvp = {
            .model = glm::rotate(glm::mat4(1.0f), _e_t * glm::radians(65.0f), glm::vec3(0.125f, 0.825f, 1.0f)),                         // This sets the model matrix to rotate around the z-axis 
            .view = glm::lookAt(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),             // This sets the view matrix to look at the origin from the z-axis
            .proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float)swapchain.extent.height, 0.1f, 10.0f),    // This sets the projection matrix to a perspective view
        };

        _mvp.proj[1][1] *= -1; // This flips the y-axis

        memcpy(uniform_data[current_frame], &_mvp, sizeof(MVP));
    }

void NovaCore::destroyUniformContext() 
    {
        report(LOGGER::VERBOSE, "Scene - Destroying Uniform Context ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            { destroyBuffer(&uniform[i]); }
    }


    ///////////////////////
    // MIPMAP GENERATION //
    ///////////////////////


static inline VkImageMemoryBarrier _getMemoryBarrier(VkImage& image, VkImageLayout& _old_layout, VkImageLayout& _new_layout, uint32_t mip_level = 1)
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

        VkCommandBuffer _cmd = createEphemeralCommand(queues.xfr.pool);

        VkImageLayout _old_layout = _IMAGE_LAYOUT_DST;
        VkImageLayout _new_layout = _IMAGE_LAYOUT_SRC;
        VkImageMemoryBarrier _barrier = _getMemoryBarrier(image, _old_layout, _new_layout);

        int _mip_width = tex_width;
        int _mip_height = tex_height;

        for (uint32_t i = 1; i < mip_levels; i++) 
            {
                report(LOGGER::VLINE, "\t\t .. Mipmap Level: %d", i);
                _setTransferBarrier(_barrier, i - 1);   
                vkCmdPipelineBarrier(_cmd, _PIPELINE_TRANSFER_BIT, _PIPELINE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
                VkImageBlit _blit = _getBlit(i, _mip_width, _mip_height);
                vkCmdBlitImage(_cmd, image, _IMAGE_LAYOUT_SRC, image, _IMAGE_LAYOUT_DST, 1, &_blit, VK_FILTER_LINEAR);
                _setReadBarrier(_barrier, i);
                vkCmdPipelineBarrier(_cmd, _PIPELINE_TRANSFER_BIT, _PIPELINE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

                if (_mip_width > 1) _mip_width /= 2;
                if (_mip_height > 1) _mip_height /= 2;
            }

        _setFinalBarrier(_barrier, mip_levels - 1);
        vkCmdPipelineBarrier(_cmd, _PIPELINE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

        char _msg[] = "Generate Mipmaps";
        flushCommandBuffer(_cmd, _msg);

        return;
    }



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

inline void NovaCore::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) 
    {
        report(LOGGER::VLINE, "\t .. Transitioning Image Layout ..");
        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(queues.xfr.pool);
        VkImageMemoryBarrier _barrier = _getMemoryBarrier(image, old_layout, new_layout, mip_lvls);
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
                _dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } 
        else 
            { report(LOGGER::ERROR, "Scene - Unsupported Layout Transition .."); return; }

        vkCmdPipelineBarrier(_ephemeral_cmd, _src_stage, _dst_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

        char _msg[] = "Transition Image Layout";
        flushCommandBuffer(_ephemeral_cmd, _msg);
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

inline void NovaCore::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height) 
    {
        report(LOGGER::VLINE, "\t .. Copying Buffer to Image ..");

        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(queues.xfr.pool);

        VkBufferImageCopy _region = _getImageCopyRegion(width, height);
        vkCmdCopyBufferToImage(_ephemeral_cmd, buffer, image, _IMAGE_LAYOUT_DST, 1, &_region);

        char _msg[] = "Copy Buffer";
        flushCommandBuffer(_ephemeral_cmd, _msg);

        return;
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

        createImage(_tex_width, _tex_height, mip_lvls, VK_SAMPLE_COUNT_1_BIT, _SRGB_FORMAT, VK_IMAGE_TILING_OPTIMAL, _IMAGE_TRANSFER_BIT, _LOCAL_DEVICE_BIT, texture.image, texture.memory);

        // Transition the image to a layout that is optimal for copying data to
        transitionImageLayout(texture.image, _SRGB_FORMAT, _IMAGE_LAYOUT_UNDEFINED, _IMAGE_LAYOUT_DST);
        copyBufferToImage(_staging.buffer, texture.image, static_cast<uint32_t>(_tex_width), static_cast<uint32_t>(_tex_height));
//        transitionImageLayout(texture.image, _SRGB_FORMAT, _IMAGE_LAYOUT_BIT, _IMAGE_LAYOUT_READ_ONLY);

        // We need to trigger the texture image to be deleted before the pipeline goes out of scope
        queues.deletion.push_fn([=]() { vkDestroyImage(logical_device, texture.image, nullptr); });
        queues.deletion.push_fn([=]() { vkFreeMemory(logical_device, texture.memory, nullptr); });

        // Clean up the staging buffer
        destroyBuffer(&_staging);

        generateMipmaps(texture.image, _SRGB_FORMAT, _tex_width, _tex_height, mip_lvls);

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

        VkImageViewCreateInfo _view_info = _getImageViewInfo(texture.image, _SRGB_FORMAT);
        VK_TRY(vkCreateImageView(logical_device, &_view_info, nullptr, &texture.view));
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
}



void NovaCore::destroyImageContext() 
    {
        report(LOGGER::VERBOSE, "Scene - Destroying Texture Context ..");

        vkDestroySampler(logical_device, texture.sampler, nullptr);
        vkDestroyImageView(logical_device, texture.view, nullptr);

    }