#pragma once
#include <vulkan/vulkan.h>
#include "./logger.h"
#include <stdio.h>
#include <cstdlib>

#define VK_TRY(x)                                                       \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            printf(" [ERROR] Vulkan: {}", err);    \
            abort();                                                    \
        }                                                               \
    } while (0)

