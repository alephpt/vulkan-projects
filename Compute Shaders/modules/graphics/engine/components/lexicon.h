#pragma once
#include <vulkan/vulkan.h>
#include "./logger.h"
#include <stdio.h>
#include <cstdlib>

#define VK_TRY(x)                                                       \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fprintf(stderr, " [ERROR] Vulkan: %s\n", err);              \
            abort();                                                    \
        }                                                               \
    } while (0)

