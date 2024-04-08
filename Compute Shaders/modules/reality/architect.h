#pragma once
#include "./scaffolds.h"
#include "./architecture/gateway.h"
#include "../../components/lexicon.h"

class Architect {
    public:
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        VkSurfaceKHR surface;
        Queues queues;
        SwapChainContext swapchain;
        VkRenderPass render_pass;
        QueuePresentContext present;
        Gateway *gateway;

        bool framebuffer_resized = false;

        Architect();
        ~Architect();

        void log();

        void setWindowExtent(VkExtent2D);

        bool deviceProvisioned(VkPhysicalDevice);
        void createPhysicalDevice();
        void createLogicalDevice();    

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
        void querySwapChainDetails();
        void constructSwapChain();
        void recreateSwapChain();
        void destroySwapChain();
        
        void createRenderPass();

        void constructImageViews();
        void createFrameBuffers();

        void createCommandPool();
        void createSyncObjects();
        
        void resetCommandBuffers();
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 

        void transitionImage();

        void constructGateway();
        void destroyGateway();
        
        void drawFrame();

    private:
        const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        FrameData& current_frame();
        int _frame_ct = 0;

        void logQueues();
        void logSwapChain();
        void logFrameData();
        void _blankContext();
        void getQueueFamilies(VkPhysicalDevice);
        void setQueueFamilyProperties(unsigned int);
        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*, uint32_t);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        VkImageViewCreateInfo createImageViewInfo(size_t);
        VkAttachmentDescription colorAttachment();
        VkCommandBufferAllocateInfo createCommandBuffers(unsigned int);
        VkRenderPassBeginInfo getRenderPassInfo(size_t);
};

