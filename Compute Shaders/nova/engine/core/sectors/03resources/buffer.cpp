#include "../../core.h"

    ////////////////////
    // BUFFER OBJECTS //
    ////////////////////

static inline VkBufferCreateInfo getBufferInfo(VkDeviceSize size, VkBufferUsageFlags usage)
    {
        report(LOGGER::VLINE, "\t\t\t .. Creating Buffer Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
        };
    }

void NovaCore::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferContext* buffer)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Buffer ..");

        VkBufferCreateInfo _buffer_info = getBufferInfo(size, usage);
        VK_TRY(vkCreateBuffer(logical_device, &_buffer_info, nullptr, &buffer->buffer));

        VkMemoryRequirements _mem_reqs;
        vkGetBufferMemoryRequirements(logical_device, buffer->buffer, &_mem_reqs);

        VkMemoryAllocateInfo _alloc_info = getMemoryAllocateInfo(_mem_reqs, properties);
        VK_TRY(vkAllocateMemory(logical_device, &_alloc_info, nullptr, &buffer->memory));

        vkBindBufferMemory(logical_device, buffer->buffer, buffer->memory, 0);

        return;
    }

static inline VkBufferCopy getBufferCopy(VkDeviceSize size)
    {
        report(LOGGER::VLINE, "\t .. Creating Buffer Copy ..");

        return {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
    }

// Copy data from one buffer to another using the Transfer Queue (if available)
// Asynchronous copy operations are possible by using the Transfer Queue for copying data to the GPU
// and the Compute Queue for running compute shaders, while the Graphics Queue is used for rendering
// and the Present Queue is used for presenting the swapchain images to the screen
void NovaCore::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, VkQueue& queue, VkCommandPool& pool)
    {
        report(LOGGER::VLINE, "\t\t .. Copying Buffer ..");
        VkCommandBuffer _ephemeral_command = createEphemeralCommand(pool);

        VkBufferCopy _copy_region = getBufferCopy(size);
        vkCmdCopyBuffer(_ephemeral_command, src_buffer, dst_buffer, 1, &_copy_region);

        char _cmd_name[] = "Copy Buffer";
        flushCommandBuffer(_ephemeral_command, _cmd_name, queue, pool);
    }
