#include "../../core.h"
#include "../00atomic/genesis.h"

#include <vector>
#include <cstring>



void NovaCore::constructStorageBuffers()
    {
        report(LOGGER::DEBUG, "Management - Constructing Storage Buffers ..");

        // Initialize Particle Systems
        std::vector<Particle> _particles(MAX_PARTICLES);
        genesis::createParticles(&_particles);
        
        // Create the Buffer
        VkDeviceSize bufferSize = sizeof(Particle) * MAX_PARTICLES;
        BufferContext stagingBuffer;
        createBuffer(bufferSize, _TRANSFER_SRC_BIT, _STAGING_PROPERTIES_BIT, &stagingBuffer);

        // Map the Buffer
        void* data;
        vkMapMemory(logical_device, stagingBuffer.memory, 0, bufferSize, 0, &data);
        memcpy(data, _particles.data(), (size_t) bufferSize);
        vkUnmapMemory(logical_device, stagingBuffer.memory);

        // Create the Buffer
        storage.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                createBuffer(bufferSize, _STORAGE_BUFFER_BIT, _LOCAL_DEVICE_BIT, &storage[i]);
                copyBuffer(stagingBuffer.buffer, storage[i].buffer, bufferSize, queues.transfer.queue, queues.transfer.pool); // TODO: We need to try and use Compute to acheive async and implement transfer queue
            }

        // Clean Up
        destroyBuffer(&stagingBuffer);
        
        return;
    }