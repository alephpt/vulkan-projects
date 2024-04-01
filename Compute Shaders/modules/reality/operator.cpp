#include "./operator.h"



    /////////////////////
    // COMMAND BUFFERS //
    /////////////////////

static VkCommandPoolCreateInfo createCommandPoolInfo(unsigned int queue_family_index)
    {
        report(LOGGER::DLINE, "\t .. Creating Command Pool Info ..");
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_family_index
        };
    }
    

static inline VkCommandBufferAllocateInfo createCommandBuffers(EngineContext* context, unsigned int n)
    {
        report(LOGGER::DLINE, "\t .. Creating Command Buffer %d ..", n);

        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = context->frames[n].command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
    }

void createCommandPool(EngineContext* context) 
    {
        report(LOGGER::DLINE, "\t .. Creating Command Pool ..");

        VkCommandPoolCreateInfo _create_info = createCommandPoolInfo(context->queues.indices.graphics_family.value());

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_TRY(vkCreateCommandPool(context->logical_device, &_create_info, nullptr, &context->frames[i].command_pool));

                VkCommandBufferAllocateInfo _alloc_info = createCommandBuffers(context, i);

                VK_TRY(vkAllocateCommandBuffers(context->logical_device, &_alloc_info, &context->frames[i].command_buffer));
            }

        return;
    }


    /////////////////////
    // SYNC STRUCTURES //
    /////////////////////

static inline VkSemaphoreCreateInfo createSemaphoreInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Semaphore Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };
    }

static inline VkFenceCreateInfo createFenceInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Fence Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
    }

void createSyncObjects(EngineContext* _context) {
    report(LOGGER::DLINE, "\t .. Creating Sync Objects ..");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkSemaphoreCreateInfo semaphore_info = createSemaphoreInfo();
        VkFenceCreateInfo fence_info = createFenceInfo();

        VK_TRY(vkCreateSemaphore(_context->logical_device, &semaphore_info, nullptr, &_context->frames[i].image_available));
        VK_TRY(vkCreateSemaphore(_context->logical_device, &semaphore_info, nullptr, &_context->frames[i].render_finished));
        VK_TRY(vkCreateFence(_context->logical_device, &fence_info, nullptr, &_context->frames[i].in_flight));
    }

    return;
}