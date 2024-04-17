#include "../engine.h"
#include <cstring>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../components/extern/stb_image.h"
const char* TEXTURE_PATH = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/modules/graphics/engine/components/extern/texture.png";

    ////////////////////////////
    // OBJECT BUFFER CREATION //
    ////////////////////////////

static const VkBufferUsageFlags _staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
static const VkMemoryPropertyFlags _staging_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

static const VkBufferUsageFlags _index_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
static const VkMemoryPropertyFlags _index_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
static const VkBufferUsageFlags _vertex_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
static const VkMemoryPropertyFlags _vertex_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

void GFXEngine::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(graphics_pipeline->vertices[0]) * graphics_pipeline->vertices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, &_staging);

        // We do this to copy the data from the CPU to the GPU
        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data); // This maps the memory to the CPU
        memcpy(data, graphics_pipeline->vertices.data(), (size_t)_buffer_size);           // This copies the data to the memory
        vkUnmapMemory(logical_device, _staging.memory);                          // This unmaps the memory from the CPU

        // We create the buffer that will be used by the GPU
        createBuffer(_buffer_size, _vertex_usage, _vertex_properties, &vertex);
        copyBuffer(_staging.buffer, vertex.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void GFXEngine::constructIndexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Index Buffer ..");

        VkDeviceSize _buffer_size = sizeof(graphics_pipeline->indices[0]) * graphics_pipeline->indices.size();
        report(LOGGER::VLINE, "\t\t .. Buffer Size: %d", _buffer_size);

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, &_staging);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, graphics_pipeline->indices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _index_usage, _index_properties, &index);
        copyBuffer(_staging.buffer, index.buffer, _buffer_size);
 
        destroyBuffer(&_staging);

        return;
    }

void GFXEngine::destroyVertexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&vertex); return; }

void GFXEngine::destroyIndexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&index); return; }

    /////////////////////////////
    // UNIFORM BUFFER CREATION //
    /////////////////////////////

static const VkBufferUsageFlags _uniform_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
static const VkMemoryPropertyFlags _uniform_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

void GFXEngine::constructUniformBuffer() 
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

void GFXEngine::updateUniformBuffer(uint32_t current_frame)
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

void GFXEngine::destroyUniformContext() 
    {
        report(LOGGER::VERBOSE, "Scene - Destroying Uniform Context ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            { destroyBuffer(&uniform[i]); }
    }

    /////////////////////////////
    // TEXTURE BUFFER CREATION //
    /////////////////////////////

static const VkBufferUsageFlags _texture_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
static const VkMemoryPropertyFlags _texture_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

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
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
    }

void GFXEngine::createTextureImage() 
    {
        report(LOGGER::VLINE, "\t .. Creating Texture Buffer ..");

        int _tex_width, _tex_height, _tex_channels;
        stbi_uc* _pixels = stbi_load(TEXTURE_PATH, &_tex_width, &_tex_height, &_tex_channels, STBI_rgb_alpha);
        VkDeviceSize _image_size = _tex_width * _tex_height * 4;

        if (!_pixels) 
            { report(LOGGER::ERROR, "Scene - Failed to load texture image .."); return; }

        return;
    }