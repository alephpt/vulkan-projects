#pragma once
#include "./atomic.h"

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainDetails querySwapChainDetails(SwapChainSupportDetails swap_chain_support, VkExtent2D windowExtent);