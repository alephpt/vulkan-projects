#pragma once
#include "./submodules/pipeline/pipeline.h"
#include "./components/lexicon.h"


class GFXEngine {
    public:
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
        Queues queues;
        SwapChainContext swapchain;
        VkSurfaceKHR surface;

        bool framebuffer_resized = false;

        GFXEngine(VkExtent2D);
        ~GFXEngine();

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
        void constructIndexBuffer();
        void createCommandBuffers();
        void createSyncObjects();
        void constructPipeline();
        
        void drawFrame();

    private:
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        VkRenderPass render_pass;
        QueuePresentContext present;
        Pipeline *pipeline;
        BufferContext vertex;
        BufferContext index;
        const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        FrameData& current_frame();
        int _frame_ct = 0;

        void logQueues();
        void logSwapChain();
        void logFrameData();
        void _blankContext();
        bool checkValidationLayerSupport();
        void createVulkanInstance();
        bool deviceProvisioned(VkPhysicalDevice);
        VkCommandBufferBeginInfo createBeginInfo();
        void getQueueFamilies(VkPhysicalDevice);
        void setQueueFamilyProperties(unsigned int);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*, uint32_t);
        VkImageViewCreateInfo createImageViewInfo(size_t);
        void recreateSwapChain();
        void destroySwapChain();
        VkRenderPassBeginInfo getRenderPassInfo(size_t);
        VkAttachmentDescription colorAttachment();
        //VkCommandBufferAllocateInfo createCommandBuffers(VkCommandPool& cmd_pool, char* name);
        void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, BufferContext*);
        void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
        void destroyBuffer(BufferContext*);
        void destroyVertexContext();
        void destroyIndexContext();
        void resetCommandBuffers();
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 
        void transitionImage();
        void destroyPipeline();
};

