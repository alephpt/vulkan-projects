#pragma once
#include "../../components/lexicon.h"

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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

const uint32_t VALIDATION_LAYER_COUNT = static_cast<uint32_t>(VALIDATION_LAYERS.size());

struct FrameData {
    VkSemaphore _image_available;
    VkSemaphore _render_finished;
    VkFence _in_flight;
    VkCommandBuffer _command_buffer;
    VkCommandPool _command_pool;
};

struct QueueFamilyIndices {
    std::optional<unsigned int> _graphics_family = -1;
    std::optional<unsigned int> _present_family = -1;
    std::optional<unsigned int> _transfer_family = -1;
    std::optional<unsigned int> _compute_family = -1;

    bool isComplete() {
        return _graphics_family >= 0 && _present_family >= 0;
    }
};

struct Queues {
    VkQueue _graphics;
    VkQueue _present;
    VkQueue _transfer;
    VkQueue _compute;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR _capabilities;
    std::vector<VkSurfaceFormatKHR> _formats;
    std::vector<VkPresentModeKHR> _present_modes;
};

struct SwapChainDetails {
    VkSurfaceFormatKHR  _surface_format;
    VkPresentModeKHR    _present_mode;
    VkExtent2D          _extent;
};

    ////////////////////
    // CORE FUNCTIONS //
    ////////////////////

// Framework Scaffolding Functions
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData);
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
bool deviceProvisioned(VkPhysicalDevice physical_gpu, VkSurfaceKHR *_surface);

// Swapchain Buffer Functions
