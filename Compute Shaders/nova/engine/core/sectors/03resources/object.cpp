#include "../../core.h"
#include <cstring>



// Do we need to move this to the graphics pipeline?
void NovaCore::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(graphics_pipeline->vertices[0]) * graphics_pipeline->vertices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _TRANSFER_SRC_BIT, _STAGING_PROPERTIES_BIT, &_staging);

        // We do this to copy the data from the CPU to the GPU
        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);            // This maps the memory to the CPU
        memcpy(data, graphics_pipeline->vertices.data(), (size_t)_buffer_size);             // This copies the data to the memory
        vkUnmapMemory(logical_device, _staging.memory);                                     // This unmaps the memory from the CPU

        // We create the buffer that will be used by the GPU
        createBuffer(_buffer_size, _VERTEX_BUFFER_BIT, _LOCAL_DEVICE_BIT, &vertex);
        copyBuffer(_staging.buffer, vertex.buffer, _buffer_size, queues.graphics, queues.gfx.pool); // TODO: I want to be able to choose which queue to use from top level

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
        copyBuffer(_staging.buffer, index.buffer, _buffer_size, queues.graphics, queues.gfx.pool); // TODO: I want to be able to choose which queue to use from top level
 
        destroyBuffer(&_staging);

        return;
    }

void NovaCore::destroyVertexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&vertex); return; }

void NovaCore::destroyIndexContext() 
    { report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); destroyBuffer(&index); return; }
