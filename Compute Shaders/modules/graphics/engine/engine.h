#pragma once
#include "./submodules/pipeline/pipeline.h"
#include "./components/lexicon.h"


class Nova {
    public:
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
        Queues queues;
        SwapChainContext swapchain;
        VkSurfaceKHR surface;

        bool framebuffer_resized = false;

        Nova(VkExtent2D);
        ~Nova();

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
        void createDescriptorSetLayout();
        void createCommandPool();
        void createTextureImage();
        void constructVertexBuffer();
        void constructIndexBuffer();
        void constructUniformBuffer();
        void constructDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        void constructGraphicsPipeline();
        void constructComputePipeline();
        
        void drawFrame();

    private:
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        VkRenderPass render_pass;
        QueuePresentContext present;
        DescriptorContext descriptor;
        Pipeline *graphics_pipeline;
        Pipeline *compute_pipeline;
        BufferContext vertex;
        BufferContext index;
        std::vector<BufferContext> uniform;
        std::vector<void*> uniform_data;
        const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        FrameData& current_frame();
        int _frame_ct = 0;

        void logQueues();
        void logSwapChain();
        void logFrameData();

        void _blankContext();
        void createVulkanInstance();

        bool checkValidationLayerSupport();
        void getQueueFamilies(VkPhysicalDevice);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        void setQueueFamilyProperties(unsigned int);

        VkMemoryAllocateInfo getMemoryAllocateInfo(VkMemoryRequirements, VkMemoryPropertyFlags);

        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*, uint32_t);
        VkImageViewCreateInfo createImageViewInfo(size_t);
        void transitionImage();
        void recreateSwapChain();

        VkRenderPassBeginInfo getRenderPassInfo(size_t);
        VkAttachmentDescription colorAttachment();

        VkCommandBufferBeginInfo createBeginInfo();
        VkCommandBufferAllocateInfo createCommandBuffersInfo(VkCommandPool&, char*);
        VkCommandBuffer createEphemeralCommand(VkCommandPool&);
        void flushCommandBuffer(VkCommandBuffer&, char*);

        void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, BufferContext*);
        void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 
        void resetCommandBuffers();
        void updateUniformBuffer(uint32_t);

        void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);
        void copyBufferToImage(VkBuffer&, VkImage&, uint32_t, uint32_t);

        bool deviceProvisioned(VkPhysicalDevice);
        void destroySwapChain();
        void destroyBuffer(BufferContext*);
        void destroyCommandContext();
        void destroyVertexContext();
        void destroyIndexContext();
        void destroyUniformContext();
        void destroyPipeline(Pipeline*);
};

