#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include "./logger.h"
#include <stdio.h>
#include <cstdlib>

#define VK_TRY(x)                                                           \
    do {                                                                    \
        VkResult err = x;                                                   \
        if (err) {                                                          \
            fprintf(stderr, " [ERROR] Vulkan: %s\n", string_VkResult(err));  \
            abort();                                                        \
        }                                                                   \
    } while (0)

