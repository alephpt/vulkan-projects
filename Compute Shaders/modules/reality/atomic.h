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
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence in_flight;
    VkCommandBuffer command_buffer;
    VkCommandPool command_pool;
};

struct QueueFamilyIndices {
    std::optional<unsigned int> graphics_family = -1;
    std::optional<unsigned int> present_family = -1;
    std::optional<unsigned int> transfer_family = -1;
    std::optional<unsigned int> compute_family = -1;

    bool isComplete() {
        return graphics_family >= 0 && present_family >= 0 && transfer_family >= 0 && compute_family >= 0;
    }
};

struct Queues {
    VkQueue graphics;
    VkQueue present;
    VkQueue compute;
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
//    std::vector<VkImageView> _image_views;
    VkFormat format;
    VkExtent2D extent;
};

struct EngineContext {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkExtent2D window_extent { 800, 600 };
    Queues queues;
    SwapChainContext swapchain;
};


    ////////////////////
    // CORE FUNCTIONS //
    ////////////////////

// Framework Scaffolding Functions
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice scanned_device, VkSurfaceKHR existing_surface);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                             VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                             void* pUserData);
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
bool deviceProvisioned(VkPhysicalDevice scanned_device, VkSurfaceKHR existing_surface);

// Swapchain Buffer Functions
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& physical_device, VkSurfaceKHR& surface);