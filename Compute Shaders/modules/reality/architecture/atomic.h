#pragma once
#include "../../../components/lexicon.h"

#include <optional>
#include <vector>


typedef void (*fnManifest)();

    /////////////////////
    // LOCAL VARIABLES //
    /////////////////////

const bool USE_VALIDATION_LAYERS = true;
constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> VALIDATION_LAYERS = 
    {
        "VK_LAYER_KHRONOS_validation"
    };

const std::vector<const char*> DEVICE_EXTENSIONS = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//        VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,
//        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };

const uint32_t VALIDATION_LAYER_COUNT = static_cast<uint32_t>(VALIDATION_LAYERS.size());

struct FrameData {
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence in_flight;
    VkCommandBuffer command_buffer;
    VkCommandPool command_pool;
};

struct QueueFamilyIndices {
    std::optional<unsigned int> graphics_family = -1;
    std::optional<unsigned int> present_family = -1;
    std::optional<unsigned int> compute_family = -1;

    bool isComplete() {
        return graphics_family >= 0 && present_family >= 0 && compute_family >= 0;
    }
};

// This could be a class that constructs queues and families dynamically
struct Queues {
    VkQueue graphics;
    VkQueue present;
    VkQueue compute;

    std::vector<VkQueueFamilyProperties> families;
    QueueFamilyIndices indices;
    std::vector<std::vector<float>> priorities;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct SwapChainDetails {
    VkSurfaceFormatKHR  surface_format;
    VkPresentModeKHR    present_mode;
    VkExtent2D          extent;
};

struct SwapChainContext {
    VkSwapchainKHR instance;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    std::vector<VkFramebuffer> framebuffers;
    VkFormat format;
    VkExtent2D extent;
    SwapChainSupportDetails support;
    SwapChainDetails details;
};

struct QueuePresentContext {
    VkSemaphore wait_semaphores[1];
    VkSemaphore signal_semaphores[1];
    VkPipelineStageFlags wait_stages[1];
    VkSubmitInfo submit_info;
    VkPresentInfoKHR present_info;
};



void destroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, 
                                                    VkDebugUtilsMessageTypeFlagsEXT, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT*, 
                                                    void*);