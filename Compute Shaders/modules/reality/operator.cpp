#include "./architect.h"


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
    

VkCommandBufferAllocateInfo EngineContext::createCommandBuffers(unsigned int n)
    {
        report(LOGGER::DLINE, "\t .. Creating Command Buffer %d ..", n);

        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = frames[n].command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
    }

void EngineContext::createCommandPool() 
    {
        report(LOGGER::DLINE, "\t .. Creating Command Pool ..");

        VkCommandPoolCreateInfo _create_info = createCommandPoolInfo(queues.indices.graphics_family.value());

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_TRY(vkCreateCommandPool(logical_device, &_create_info, nullptr, &frames[i].command_pool));

                VkCommandBufferAllocateInfo _alloc_info = createCommandBuffers(i);

                VK_TRY(vkAllocateCommandBuffers(logical_device, &_alloc_info, &frames[i].command_buffer));
            }

        return;
    }

static VkCommandBufferBeginInfo createBeginInfo() {
    return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
        };
}

VkRenderPassBeginInfo EngineContext::getRenderPassInfo(size_t i)
    {
        return {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = render_pass,
                .framebuffer = frames[i].frame_buffer,
                .renderArea = {
                    .offset = {0, 0},
                    .extent = window_extent
                },
                .clearValueCount = 1,
                .pClearValues = &CLEAR_COLOR
            };
    }

void EngineContext::recordCommandBuffers() 
    {
        report(LOGGER::DLINE, "\t .. Recording Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkCommandBufferBeginInfo _begin_info = createBeginInfo();

                VK_TRY(vkBeginCommandBuffer(frames[i].command_buffer, &_begin_info));

                VkRenderPassBeginInfo _render_pass_info = getRenderPassInfo(i);

                vkCmdBeginRenderPass(frames[i].command_buffer, &_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(frames[i].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

                VkBuffer _vertex_buffers[] = { vertex_buffer };
                VkDeviceSize _offsets[] = { 0 };
                vkCmdBindVertexBuffers(frames[i].command_buffer, 0, 1, _vertex_buffers, _offsets);

                vkCmdBindIndexBuffer(frames[i].command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(frames[i].command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

                vkCmdEndRenderPass(frames[i].command_buffer);

                VK_TRY(vkEndCommandBuffer(frames[i].command_buffer));
            }

        return;
    }

void EngineContext::resetCommandBuffers() 
    {
        report(LOGGER::DLINE, "\t .. Resetting Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_TRY(vkResetCommandBuffer(frames[i].command_buffer, 0));
            }

        return;
    }


    /////////////////////
    // SYNC STRUCTURES //
    /////////////////////

static VkSemaphoreCreateInfo createSemaphoreInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Semaphore Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };
    }

static VkFenceCreateInfo createFenceInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Fence Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
    }

void EngineContext::createSyncObjects() 
    {
        report(LOGGER::DLINE, "\t .. Creating Sync Objects ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkSemaphoreCreateInfo semaphore_info = createSemaphoreInfo();
            VkFenceCreateInfo fence_info = createFenceInfo();

            VK_TRY(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &frames[i].image_available));
            VK_TRY(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &frames[i].render_finished));
            VK_TRY(vkCreateFence(logical_device, &fence_info, nullptr, &frames[i].in_flight));
        }

        return;
    }
