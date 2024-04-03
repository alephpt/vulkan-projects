#pragma once
#include "./architecture/atomic.h"

bool checkValidationLayerSupport();
void createVulkanInstance(VkInstance *instance);
void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger);