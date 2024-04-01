#include "illuminati.h"


VkPresentInfoKHR getPresentInfoKHR(VkSemaphore* signal_semaphores, VkSwapchainKHR* swapchains, uint32_t* image_indices) {
    return {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = image_indices
    };
}

VkSubmitInfo getSubmitInfo(QueuePresentContext* present, VkCommandBuffer* command_buffer) {
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
