#include "../../core.h"


    /////////////////////
    // COMMAND BUFFERS //
    /////////////////////

VkCommandBufferBeginInfo NovaCore::createBeginInfo()
    {
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr
            };
    }
    
static inline VkCommandPoolCreateInfo _createCommandPoolInfo(unsigned int queue_family_index, char* name)
    {
        report(LOGGER::VLINE, "\t\t .. Creating %s Command Pool Info on Queue %d ..", name, queue_family_index);
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue_family_index
            };
    }
    
void NovaCore::createCommandPool() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Pool ..");

        {
            char name[] = "Graphics";
            VkCommandPoolCreateInfo _gfx_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.graphics_family.value(), name);

            VK_TRY(vkCreateCommandPool(logical_device, &_gfx_cmd_pool_create_info, nullptr, &queues.command_pool));
        }


        {
            char name[] = "Transfer";
            VkCommandPoolCreateInfo _xfr_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.transfer_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_xfr_cmd_pool_create_info, nullptr, &queues.transfer.pool));
        }

        {
            char name[] = "Compute";
            VkCommandPoolCreateInfo _cmp_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.compute_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_cmp_cmd_pool_create_info, nullptr, &queues.compute.pool));
        }

        return;
    }

inline VkCommandBufferAllocateInfo NovaCore::createCommandBuffersInfo(VkCommandPool& cmd_pool, char* name)
    {
        report(LOGGER::VLINE, "\t\t\t\t .. Creating %s Command Buffer Info  ..", name);

        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = cmd_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
    }

void NovaCore::createCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            char name[32];
            sprintf(name, "Graphics %d", i);
            VkCommandBufferAllocateInfo _gfx_cmd_buf_alloc_info = createCommandBuffersInfo(queues.command_pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_gfx_cmd_buf_alloc_info, &frames[i].command_buffer));
        }

        {
            char name[] = "Transfer";
            VkCommandBufferAllocateInfo _xfr_cmd_buf_alloc_info = createCommandBuffersInfo(queues.transfer.pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_xfr_cmd_buf_alloc_info, &queues.transfer.buffer));
        }

        for (size_t i = 0; i < MAX_COMPUTE_QUEUES; i++)
        {
            char name[] = "Compute";
            VkCommandBufferAllocateInfo _cmp_cmd_buf_alloc_info = createCommandBuffersInfo(queues.compute.pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_cmp_cmd_buf_alloc_info, &compute[i].command_buffer));
        }

        return;
    }

VkCommandBuffer NovaCore::createEphemeralCommand(VkCommandPool& pool) 
    {
        report(LOGGER::VLINE, "\t\t\t .. Creating Ephemeral Command Buffer ..");

        VkCommandBuffer _buffer;

        char name[] = "Ephemeral";
        VkCommandBufferAllocateInfo _tmp_alloc_info = createCommandBuffersInfo(pool, name);

        VK_TRY(vkAllocateCommandBuffers(logical_device, &_tmp_alloc_info, &_buffer));

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(_buffer, &_begin_info));

        return _buffer;
    }

static inline VkSubmitInfo _createSubmitInfo(VkCommandBuffer* cmd)
    {
        report(LOGGER::VLINE, "\t .. Creating Submit Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = cmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr
        };
    }

void NovaCore::flushCommandBuffer(VkCommandBuffer& buf, char* name, VkQueue& submit_queue, VkCommandPool& pool)
    {
        report(LOGGER::VLINE, "\t .. Ending %s Command Buffer ..", name);

        VK_TRY(vkEndCommandBuffer(buf));

        // Submit the command buffer
        VkSubmitInfo _submit_info = _createSubmitInfo(&buf);
        VK_TRY(vkQueueSubmit(submit_queue, 1, &_submit_info, VK_NULL_HANDLE));
        VK_TRY(vkQueueWaitIdle(submit_queue));

        // Free the command buffer
        vkFreeCommandBuffers(logical_device, pool, 1, &buf);

        return;
    }

void NovaCore::destroyCommandContext()
    {
        report(LOGGER::VERBOSE, "Management - Destroying Semaphores, Fences and Command Pools ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(logical_device, frames[i].image_available, nullptr);
                vkDestroySemaphore(logical_device, frames[i].render_finished, nullptr);
                vkDestroyFence(logical_device, frames[i].in_flight, nullptr);
            }

        vkDestroyCommandPool(logical_device, queues.command_pool, nullptr);
    }


    //////////////////////////////
    // COMMAND BUFFER RECORDING //
    //////////////////////////////

static inline VkViewport getViewport(VkExtent2D extent)
    {
        return {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
    }

static inline VkRect2D getScissor(VkExtent2D extent)
    {
        return {
            .offset = {0, 0},
            .extent = extent
        };
    }

void NovaCore::recordCommandBuffers(VkCommandBuffer& command_buffer, uint32_t i) 
    {
        //report(LOGGER::VLINE, "\t .. Recording Command Buffer %d ..", i);

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(command_buffer, &_begin_info));

        VkRenderPassBeginInfo _render_pass_info = getRenderPassInfo(i);
        vkCmdBeginRenderPass(command_buffer, &_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->instance);

        VkViewport _viewport = getViewport(swapchain.details.extent);
        vkCmdSetViewport(command_buffer, 0, 1, &_viewport);

        VkRect2D _scissor = getScissor(swapchain.details.extent);
        vkCmdSetScissor(command_buffer, 0, 1, &_scissor);

        VkBuffer _vertex_buffers[] = {vertex.buffer};
        VkDeviceSize _offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, _vertex_buffers, _offsets);
        vkCmdBindIndexBuffer(command_buffer, index.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->layout, 0, 1, &descriptor.sets[_frame_ct], 0, nullptr);

        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(graphics_pipeline->indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(command_buffer);

        VK_TRY(vkEndCommandBuffer(command_buffer));

        return;
    }

void NovaCore::resetCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Resetting Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_TRY(vkResetCommandBuffer(frames[i].command_buffer, 0));
        }

        return;
    }

