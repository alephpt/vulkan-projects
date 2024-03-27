#pragma once
#include <vulkan/vulkan.h>
#include <fmt/core.h>

#define VK_TRY(x)                                                       \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fmt::print(" [ERROR] Vulkan: {}", err);    \
            abort();                                                    \
        }                                                               \
    } while (0)

