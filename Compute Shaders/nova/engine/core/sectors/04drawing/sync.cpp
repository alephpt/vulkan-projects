#include "../../core.h"

#include <SDL2/SDL_timer.h>

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

void NovaCore::createSyncObjects() 
    {
        report(LOGGER::VLINE, "\t .. Creating Sync Objects ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                VkSemaphoreCreateInfo frames_semaphore_info = createSemaphoreInfo();
                VkFenceCreateInfo frames_fence_info = createFenceInfo();

                VK_TRY(vkCreateSemaphore(logical_device, &frames_semaphore_info, nullptr, &frames[i].image_available));
                VK_TRY(vkCreateSemaphore(logical_device, &frames_semaphore_info, nullptr, &frames[i].render_finished));
                VK_TRY(vkCreateFence(logical_device, &frames_fence_info, nullptr, &frames[i].in_flight));
                
                VkSemaphoreCreateInfo computes_semaphore_info = createSemaphoreInfo();
                VkFenceCreateInfo computes_fence_info = createFenceInfo();

                VK_TRY(vkCreateSemaphore(logical_device, &computes_semaphore_info, nullptr, &computes[i].finished));
                VK_TRY(vkCreateFence(logical_device, &computes_fence_info, nullptr, &computes[i].in_flight));
            }

        return;
    }

void NovaCore::syncClock()
    {
        double current_time = SDL_GetTicks64() / 1000.0;
        last_frame_time = current_time - last_time;
        last_time = current_time;
    }