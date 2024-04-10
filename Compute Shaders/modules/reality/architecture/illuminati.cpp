#include "../architect.h"

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

static inline VkSubmitInfo getSubmitInfo(QueuePresentContext* present, VkCommandBuffer* command_buffer, VkSemaphore* _signal_semaphore, VkSemaphore* _wait_semaphore, VkPipelineStageFlags* _wait_stages) 
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

void Architect::drawFrame() 
    {
        report(LOGGER::VLINE, "\t .. Drawing Frame %d ..", _frame_ct);

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

        report(LOGGER::VLINE, "\t .. Acquired Image Index: %d", _image_index);

        VkCommandBuffer _command_buffer = queues.cmd_buf_gfx;

        VK_TRY(vkResetFences(logical_device, 1, &current_frame().in_flight));
        VK_TRY(vkResetCommandBuffer(_command_buffer, 0));

        recordCommandBuffers(_command_buffer, _image_index);

        present.submit_info = {};
        VkSemaphore _wait_semaphores[] = { current_frame().image_available };
        VkSemaphore _signal_semaphores[] = { current_frame().render_finished };
        VkPipelineStageFlags _wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        present.submit_info = getSubmitInfo(&present, &_command_buffer, _signal_semaphores, _wait_semaphores, _wait_stages);

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

void Architect::transitionImage() {
    
}