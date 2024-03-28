#pragma once
#include "./atomic.h"

SwapChainDetails querySwapChainDetails(SwapChainSupportDetails swap_chain_support, VkExtent2D windowExtent);
void constructSwapChain(SwapChainDetails swap_chain_details, SwapChainSupportDetails swap_chain_support, EngineContext *context);