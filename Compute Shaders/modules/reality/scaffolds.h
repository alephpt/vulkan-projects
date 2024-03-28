#pragma once
#include "./atomic.h"

bool checkValidationLayerSupport();
void createVulkanInstance(VkInstance *instance);
void createDebugMessenger(VkInstance *instance, VkDebugUtilsMessengerEXT *_debug_messenger);
bool createDeviceQueues(VkPhysicalDevice physical_gpu, VkSurfaceKHR *_surface);
void createPhysicalDevice(VkInstance instance, VkPhysicalDevice *physical_gpu, VkSurfaceKHR *_surface);
void createLogicalDevice(VkPhysicalDevice physical_gpu, VkDevice logical_gpu, struct Queues& _queues, VkSurfaceKHR *_surface);