#include "../engine.h"



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

void GFXEngine::recordCommandBuffers(VkCommandBuffer& command_buffer, uint32_t i) 
    {
        //report(LOGGER::VLINE, "\t .. Recording Command Buffer %d ..", i);

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(command_buffer, &_begin_info));

        VkRenderPassBeginInfo _render_pass_info = getRenderPassInfo(i);
        vkCmdBeginRenderPass(command_buffer, &_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->instance);

        VkViewport _viewport = getViewport(swapchain.extent);
        vkCmdSetViewport(command_buffer, 0, 1, &_viewport);

        VkRect2D _scissor = getScissor(swapchain.extent);
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

void GFXEngine::resetCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Resetting Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_TRY(vkResetCommandBuffer(frames[i].cmd.buffer, 0));
        }

        VK_TRY(vkResetCommandBuffer(queues.xfr.buffer, 0));
        VK_TRY(vkResetCommandBuffer(queues.cmp.buffer, 0));

        return;
    }




    //////////////////////
    // RENDER UTILITIES //
    //////////////////////

static inline VkPresentInfoKHR getPresentInfoKHR(VkSemaphore* signal_semaphores, VkSwapchainKHR* swapchains, uint32_t* image_indices) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = image_indices
        };
    }

static inline VkSemaphoreSubmitInfo getSemaphoreSubmitInfo(VkSemaphore* semaphore, VkPipelineStageFlags stage_mask) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = *semaphore,
            .value = 1,
            .stageMask = stage_mask,
            .deviceIndex = 0
        };
    }

static inline VkCommandBufferSubmitInfo getCommandBufferSubmitInfo(VkCommandBuffer* command_buffer) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = *command_buffer,
            .deviceMask = 0,
        };
    }

static inline VkSubmitInfo getSubmitInfo(VkCommandBuffer* command_buffer, VkSemaphore* _signal_semaphore, VkSemaphore* _wait_semaphore, VkPipelineStageFlags* _wait_stages) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = _wait_semaphore ? (uint32_t)1 : 0,
            .pWaitSemaphores = _wait_semaphore,
            .pWaitDstStageMask = _wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = command_buffer,
            .signalSemaphoreCount = _signal_semaphore ? (uint32_t)1 : 0,
            .pSignalSemaphores = _signal_semaphore
        };
    }

static inline VkSubmitInfo2 getSubmitInfo2(VkSemaphoreSubmitInfo* wait_info, VkCommandBufferSubmitInfo* command_info, VkSemaphoreSubmitInfo* signal_info) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = wait_info,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = command_info,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = signal_info
        };
    }


    /////////////////
    // ACTUAL DRAW //
    /////////////////

void GFXEngine::drawFrame() 
    {
        //report(LOGGER::VLINE, "\t .. Drawing Frame %d ..", _frame_ct);

        VK_TRY(vkWaitForFences(logical_device, 1, &current_frame().in_flight, VK_TRUE, UINT64_MAX));
        current_frame().deletion_queue.flush();

        //log();
        uint32_t _image_index;
        VkResult result = vkAcquireNextImageKHR(
                            logical_device, 
                            swapchain.instance, 
                            UINT64_MAX, 
                            current_frame().image_available, 
                            VK_NULL_HANDLE, 
                            &_image_index
                        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapChain();
                return;
            }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                report(LOGGER::ERROR, "Failed to acquire swap chain image!");
                VK_TRY(result);
            }

        VkCommandBuffer _command_buffer = current_frame().cmd.buffer;
        updateUniformBuffer(_frame_ct);

        VK_TRY(vkResetFences(logical_device, 1, &current_frame().in_flight));
        VK_TRY(vkResetCommandBuffer(_command_buffer, 0));

        recordCommandBuffers(_command_buffer, _image_index);

        present.submit_info = {};
        VkSemaphore _wait_semaphores[] = { current_frame().image_available };
        VkSemaphore _signal_semaphores[] = { current_frame().render_finished };
        VkPipelineStageFlags _wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        present.submit_info = getSubmitInfo(&_command_buffer, _signal_semaphores, _wait_semaphores, _wait_stages);

        VK_TRY(vkQueueSubmit(queues.graphics, 1, &present.submit_info, current_frame().in_flight));

        VkSwapchainKHR _swapchains[] = { swapchain.instance };
        present.present_info = {};
        present.present_info = getPresentInfoKHR(_signal_semaphores, _swapchains, &_image_index);

        result = vkQueuePresentKHR(queues.present, &present.present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized)
            {
                recreateSwapChain();
                framebuffer_resized = false;
            }
        else if (result != VK_SUCCESS)
            {
                report(LOGGER::ERROR, "Failed to present swap chain image!");
                VK_TRY(result);
            }

        _frame_ct = (_frame_ct + 1) % MAX_FRAMES_IN_FLIGHT;

        return;
    }


    //////////////////////
    // TRANSITION IMAGE //
    //////////////////////

void GFXEngine::transitionImage() {
    
}