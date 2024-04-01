#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"

VkSubmitInfo getSubmitInfo(QueuePresentContext*, VkCommandBuffer*);
VkPresentInfoKHR getPresentInfoKHR(VkSemaphore*, VkSwapchainKHR*, uint32_t*);