#include "../engine.h"
#include <cstring>
#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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

        VkDeviceSize _buffer_size = sizeof(pipeline->vertices[0]) * pipeline->vertices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, &_staging);

        // We do this to copy the data from the CPU to the GPU
        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data); // This maps the memory to the CPU
        memcpy(data, pipeline->vertices.data(), (size_t)_buffer_size);           // This copies the data to the memory
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

        VkDeviceSize _buffer_size = sizeof(pipeline->indices[0]) * pipeline->indices.size();
        report(LOGGER::VLINE, "\t\t .. Buffer Size: %d", _buffer_size);

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, &_staging);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, pipeline->indices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _index_usage, _index_properties, &index);
        copyBuffer(_staging.buffer, index.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void GFXEngine::destroyVertexContext() 
    {
        report(LOGGER::VERBOSE, "GFXEngine - Destroying Vertex Context ..");

        destroyBuffer(&vertex);

        return;
    }


void GFXEngine::destroyIndexContext() 
    {
        report(LOGGER::VERBOSE, "GFXEngine - Destroying Vertex Context ..");

        destroyBuffer(&index);

        return;
    }

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
        report(LOGGER::VERBOSE, "GFXEngine - Destroying Uniform Context ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            { destroyBuffer(&uniform[i]); }
    }