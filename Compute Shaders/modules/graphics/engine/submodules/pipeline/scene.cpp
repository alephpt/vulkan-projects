#include "../../engine.h"
#include <cstring>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../../components/extern/stb_image.h"
const char* TEXTURE_PATH = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/modules/graphics/engine/components/extern/texture.png";

// Do we need to move this to the graphics pipeline?

    ////////////////////////////
    // OBJECT BUFFER CREATION //
    ////////////////////////////

static const VkBufferUsageFlags _TRANSFER_SRC_BIT = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
static const VkBufferUsageFlags _BUFFER_INDEX_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
static const VkBufferUsageFlags _VERTEX_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
static const VkBufferUsageFlags _IMAGE_SAMPLED_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
static const VkMemoryPropertyFlags _STAGING_PROPERTIES_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
static const VkMemoryPropertyFlags _LOCAL_DEVICE_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
static const VkImageLayout _IMAGE_LAYOUT_BIT = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
static const VkImageLayout _IMAGE_LAYOUT_UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED;
static const VkImageLayout _IMAGE_LAYOUT_READ_ONLY = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        

void Nova::constructVertexBuffer() 
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

void Nova::constructIndexBuffer() 
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

        createBuffer(_buffer_size, _BUFFER_INDEX_BIT, _LOCAL_DEVICE_BIT, &index);
        copyBuffer(_staging.buffer, index.buffer, _buffer_size);
 
        destroyBuffer(&_staging);

        return;
    }

void Nova::destroyVertexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&vertex); return; }

void Nova::destroyIndexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&index); return; }

    /////////////////////////////
    // UNIFORM BUFFER CREATION //
    /////////////////////////////

static const VkBufferUsageFlags _uniform_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
static const VkMemoryPropertyFlags _uniform_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

void Nova::constructUniformBuffer() 
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

void Nova::updateUniformBuffer(uint32_t current_frame)
    {
        static auto _s_t = std::chrono::high_resolution_clock::now();

        auto _c_t = std::chrono::high_resolution_clock::now();
        float _e_t = std::chrono::duration<float, std::chrono::seconds::period>(_c_t - _s_t).count();

        MVP _mvp = {
            .model = glm::rotate(glm::mat4(1.0f), _e_t * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),                         // This sets the model matrix to rotate around the z-axis
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),             // This sets the view matrix to look at the origin from the z-axis
            .proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float)swapchain.extent.height, 0.1f, 10.0f),    // This sets the projection matrix to a perspective view
        };

        _mvp.proj[1][1] *= -1; // This flips the y-axis

        memcpy(uniform_data[current_frame], &_mvp, sizeof(MVP));
    }

void Nova::destroyUniformContext() 
    {
        report(LOGGER::VERBOSE, "Scene - Destroying Uniform Context ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            { destroyBuffer(&uniform[i]); }
    }

    /////////////////////////////
    // TEXTURE BUFFER CREATION //
    /////////////////////////////


static inline VkImageCreateInfo createImageInfo(int w, int h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = { .width = (uint32_t)w, .height = (uint32_t)h, .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = _IMAGE_LAYOUT_UNDEFINED,
        };
    }


static inline VkImageMemoryBarrier getMemoryBarrier(VkImage& image, VkImageLayout& _old_layout, VkImageLayout& _new_layout) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = _old_layout,
            .newLayout = _new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = { 
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
                .baseMipLevel = 0, 
                .levelCount = 1, 
                .baseArrayLayer = 0, 
                .layerCount = 1 
                }
        };
    }

inline void Nova::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) 
    {
        report(LOGGER::VLINE, "\t .. Transitioning Image Layout ..");
        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(queues.xfr.pool);
        VkImageMemoryBarrier _barrier = getMemoryBarrier(image, old_layout, new_layout);
        VkPipelineStageFlags _src_stage, _dst_stage;

        if (old_layout == _IMAGE_LAYOUT_UNDEFINED && new_layout == _IMAGE_LAYOUT_BIT) 
            {
                _barrier.srcAccessMask = 0;
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                _src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                _dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } 
        else if (old_layout == _IMAGE_LAYOUT_BIT && new_layout == _IMAGE_LAYOUT_READ_ONLY) 
            {
                _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                _src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                _dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } 
        else 
            { report(LOGGER::ERROR, "Scene - Unsupported Layout Transition .."); return; }

        vkCmdPipelineBarrier(
            _ephemeral_cmd, 
            _src_stage, 
            _dst_stage, 
            0, 
            0, 
            nullptr, 
            0, 
            nullptr, 
            1, 
            &_barrier
        );

        char _msg[] = "Transition Image Layout";
        flushCommandBuffer(_ephemeral_cmd, _msg);
    }

static inline VkBufferImageCopy getImageCopyRegion(uint32_t width, uint32_t height) 
    {
        return {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
            .imageOffset = { .x = 0, .y = 0, .z = 0 },
            .imageExtent = { .width = width, .height = height, .depth = 1 }
        };
    }

inline void Nova::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height) 
    {
        report(LOGGER::VLINE, "\t .. Copying Buffer to Image ..");

        VkCommandBuffer _ephemeral_cmd = createEphemeralCommand(queues.xfr.pool);

        VkBufferImageCopy _region = getImageCopyRegion(width, height);
        vkCmdCopyBufferToImage(_ephemeral_cmd, buffer, image, _IMAGE_LAYOUT_BIT, 1, &_region);

        char _msg[] = "Copy Buffer";
        flushCommandBuffer(_ephemeral_cmd, _msg);

        return;
    }

void Nova::createTextureImage() 
    {
        report(LOGGER::VLINE, "\t .. Creating Texture Buffer ..");

        // Load the texture image
        int _tex_width, _tex_height, _tex_channels;
        stbi_uc* _pixels = stbi_load(TEXTURE_PATH, &_tex_width, &_tex_height, &_tex_channels, STBI_rgb_alpha);
        VkDeviceSize _image_size = _tex_width * _tex_height * 4;

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

        // Create the image
        const VkFormat _FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
        const VkImageTiling _TILING = VK_IMAGE_TILING_OPTIMAL;

        VkImageCreateInfo _image_info = createImageInfo(_tex_width, _tex_height, _FORMAT, _TILING, _IMAGE_SAMPLED_BIT);
        VK_TRY(vkCreateImage(logical_device, &_image_info, nullptr, &graphics_pipeline->texture_image));

        VkMemoryRequirements _mem_reqs;
        vkGetImageMemoryRequirements(logical_device, graphics_pipeline->texture_image, &_mem_reqs);

        VkMemoryAllocateInfo _alloc_info = getMemoryAllocateInfo(_mem_reqs, _LOCAL_DEVICE_BIT);
        VK_TRY(vkAllocateMemory(logical_device, &_alloc_info, nullptr, &graphics_pipeline->texture_image_memory));

        vkBindImageMemory(logical_device, graphics_pipeline->texture_image, graphics_pipeline->texture_image_memory, 0);

        // Transition the image to a layout that is optimal for copying data to
        transitionImageLayout(graphics_pipeline->texture_image, _FORMAT, _IMAGE_LAYOUT_UNDEFINED, _IMAGE_LAYOUT_BIT);
        copyBufferToImage(_staging.buffer, graphics_pipeline->texture_image, static_cast<uint32_t>(_tex_width), static_cast<uint32_t>(_tex_height));
        transitionImageLayout(graphics_pipeline->texture_image, _FORMAT, _IMAGE_LAYOUT_BIT, _IMAGE_LAYOUT_READ_ONLY);

        // We need to trigger the texture image to be deleted before the pipeline goes out of scope
        queues.deletion.push_fn([=]() { vkDestroyImage(logical_device, graphics_pipeline->texture_image, nullptr); });
        queues.deletion.push_fn([=]() { vkFreeMemory(logical_device, graphics_pipeline->texture_image_memory, nullptr); });

        // Clean up the staging buffer
        destroyBuffer(&_staging);

        return;
    }