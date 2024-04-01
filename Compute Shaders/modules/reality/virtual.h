#pragma once
#include "./atomic.h"

SwapChainDetails querySwapChainDetails(SwapChainSupportDetails, VkExtent2D);
void constructSwapChain(SwapChainDetails, SwapChainSupportDetails, EngineContext*);
void constructImageViews(EngineContext*);
void createFrameBuffers(EngineContext*);
void destroySwapChain(EngineContext*);
void recreateSwapChain(EngineContext*);
