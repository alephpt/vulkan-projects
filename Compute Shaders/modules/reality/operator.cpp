#include "./operator.h"


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
    

void createCommandPool(EngineContext* context) 
    {
        report(LOGGER::DLINE, "\t .. Creating Command Pool ..");

        VkCommandPoolCreateInfo _create_info = createCommandPoolInfo(context->queues.indices.graphics_family.value());

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_TRY(vkCreateCommandPool(context->logical_device, &_create_info, nullptr, &context->frames[MAX_FRAMES_IN_FLIGHT].command_pool));

                VkCommandBufferAllocateInfo _alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .commandPool = context->frames[MAX_FRAMES_IN_FLIGHT].command_pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1
                };

                VK_TRY(vkAllocateCommandBuffers(context->logical_device, &_alloc_info, &context->frames[MAX_FRAMES_IN_FLIGHT].command_buffer));
            }

        return;
    }

void createCommandBuffers(EngineContext* context)
    {
        report(LOGGER::DLINE, "\t .. Creating Command Buffers ..");
        return;
    }