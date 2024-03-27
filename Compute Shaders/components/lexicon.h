#pragma once
#include <vulkan/vulkan.h>

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fmt::print(" [ERROR] Vulkan: {}", string_VkResult(err));    \
            abort();                                                    \
        }                                                               \
    } while (0)

