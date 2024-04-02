#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"

class EngineContext {
    public:
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        VkSurfaceKHR surface;
        VkExtent2D window_extent { 800, 600 };
        Queues queues;
        SwapChainContext swapchain;
        VkRenderPass render_pass;
        QueuePresentContext present;

        EngineContext();
        ~EngineContext();

        void setWindowExtent(VkExtent2D);


        void logSwapChain();
        void log();

        bool deviceProvisioned(VkPhysicalDevice);
        void createPhysicalDevice();
        void createLogicalDevice();    

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
        void querySwapChainDetails();
        void constructSwapChain();
        void constructImageViews();
        void createFrameBuffers();
        void destroySwapChain();
        void recreateSwapChain();
        void createRenderPass();

    private:
        void _blankContext();
        void getQueueFamilies(VkPhysicalDevice);
        void setQueueFamilyProperties(unsigned int);
        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        VkImageViewCreateInfo createImageViewInfo(size_t);
        VkAttachmentDescription colorAttachment();
};

