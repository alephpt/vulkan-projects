#include "../architect.h"

static inline VkPresentInfoKHR getPresentInfoKHR(VkSemaphore* signal_semaphores, VkSwapchainKHR* swapchains, uint32_t* image_indices) {
    return {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = image_indices
    };
}

static inline VkSubmitInfo getSubmitInfo(QueuePresentContext* present, VkCommandBuffer* command_buffer) {
    return {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = present->wait_semaphores,
        .pWaitDstStageMask = present->wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = present->signal_semaphores
    };
}

void Architect::drawFrame() 
    {
        report(LOGGER::INFO, "Matrix - Drawing Frame ..");

        vkWaitForFences(logical_device, 1, &current_frame().in_flight, VK_TRUE, UINT64_MAX);
        vkResetFences(logical_device, 1, &current_frame().in_flight);

        uint32_t _image_index;
        VkResult res = vkAcquireNextImageKHR(
                            logical_device, 
                            swapchain.instance, 
                            UINT64_MAX, 
                            current_frame().image_available, 
                            VK_NULL_HANDLE, 
                            &_image_index
                        );

        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
            printf("Failed to acquire swap chain image!\n");
            return;
        }

        vkResetCommandBuffer(current_frame().command_buffer, 0);
        recordCommandBuffers(_image_index);

        // We don't have to store these
        present.wait_semaphores[0] = current_frame().image_available;
        present.signal_semaphores[0] = current_frame().render_finished;
        present.wait_stages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        present.submit_info = getSubmitInfo(&present, &current_frame().command_buffer);

        VK_TRY(vkQueueSubmit(queues.graphics, 1, &present.submit_info, current_frame().in_flight));

        VkSwapchainKHR _swapchains[] = { swapchain.instance };
        present.present_info = getPresentInfoKHR(present.signal_semaphores, _swapchains, &_image_index);

        res = vkQueuePresentKHR(queues.present, &present.present_info);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
            framebuffer_resized = false;
            recreateSwapChain();
        } else if (res != VK_SUCCESS) {
            printf("Failed to present swap chain image!\n");
        }

        _frame_ct = (_frame_ct + 1) % MAX_FRAMES_IN_FLIGHT;

        return;
    }

