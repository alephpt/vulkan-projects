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
        VertexContext vertex;
        bool framebuffer_resized = false;

        Architect();
        ~Architect();

        void log();

        void setWindowExtent(VkExtent2D);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
        void querySwapChainDetails();
        void createPhysicalDevice();
        void createLogicalDevice(); 
        void constructSwapChain();
        void constructImageViews();
        void createFrameBuffers();
        void createRenderPass();
        void createCommandPool();
        void constructVertexBuffer();
        void createCommandBuffers();
        void createSyncObjects();
        void constructGateway();
        
        void drawFrame();

    private:
        const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        FrameData& current_frame();
        int _frame_ct = 0;

        void logQueues();
        void logSwapChain();
        void logFrameData();
        void _blankContext();
        bool deviceProvisioned(VkPhysicalDevice);
        void getQueueFamilies(VkPhysicalDevice);
        void setQueueFamilyProperties(unsigned int);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*, uint32_t);
        VkImageViewCreateInfo createImageViewInfo(size_t);
        void recreateSwapChain();
        void destroySwapChain();
        VkRenderPassBeginInfo getRenderPassInfo(size_t);
        VkAttachmentDescription colorAttachment();
        VkCommandBufferAllocateInfo createCommandBuffers(unsigned int);
        void destroyVertexContext();
        void resetCommandBuffers();
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 
        void transitionImage();
        void destroyGateway();
};

