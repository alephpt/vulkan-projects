#pragma once
#include "./sectors/pipeline/pipeline.h"
#include "./components/utility/lexicon.h"

//const _TILING_OPTIMAL = VK_IMAGE_TILING_OPTIMAL


class NovaCore {
    public:
        VkInstance instance;
        VkDevice logical_device;
        Queues queues;
        SwapChainContext swapchain;
        VkSurfaceKHR surface;

        bool framebuffer_resized = false;

        NovaCore(VkExtent2D);
        ~NovaCore();

        void log();

        void setWindowExtent(VkExtent2D);

        void querySwapChainDetails();
        void createPhysicalDevice();
        void createLogicalDevice(); 
        void constructSwapChain();
        void constructImageViews();
        void createFrameBuffers();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createCommandPool();
        void createColorResources();
        void createDepthResources(); 
        void createTextureImage();
        void createTextureImageView();
        void constructTextureSampler();
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
        VkPhysicalDevice physical_device;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        VkRenderPass render_pass;
        QueuePresentContext present;
        DescriptorContext descriptor;   // TODO: Create a createNewDescriptor function (and combine with uniform?)
        Pipeline *graphics_pipeline;    // TODO: Dynamically allocate pipelines with a createNewPipeline function that takes a type and/or shader file
        Pipeline *compute_pipeline;
        BufferContext vertex;           // TODO: Combine vertex and index into a single Object Buffer
        BufferContext index;            //       and create a createNewObject function
        ImageContext color;
        ImageContext depth;
        ImageContext texture;         // TODO: Create a createNewTexture function
        std::vector<BufferContext> uniform;
        std::vector<void*> uniform_data;
        const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        FrameData& current_frame();
        int _frame_ct = 0;
        VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;
        uint32_t mip_lvls = 1;

        void logQueues();
        void logSwapChain();
        void logFrameData();

        void _blankContext();
        void createVulkanInstance();

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
        bool deviceProvisioned(VkPhysicalDevice);
        bool checkValidationLayerSupport();
        void getQueueFamilies(VkPhysicalDevice);
        VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t);
        void setQueueFamilyProperties(unsigned int);

        VkMemoryAllocateInfo getMemoryAllocateInfo(VkMemoryRequirements, VkMemoryPropertyFlags);

        void createSwapchainInfoKHR(VkSwapchainCreateInfoKHR*, uint32_t);
        VkImageViewCreateInfo createImageViewInfo(VkImage, VkFormat, VkImageAspectFlags, uint32_t);
        void transitionImage();
        void recreateSwapChain();

        VkFormat findDepthFormat(VkImageTiling);
        VkAttachmentDescription getDepthAttachment();
        VkRenderPassBeginInfo getRenderPassInfo(size_t);
        VkAttachmentDescription getColorAttachment();

        VkCommandBufferBeginInfo createBeginInfo();
        VkCommandBufferAllocateInfo createCommandBuffersInfo(VkCommandPool&, char*);
        VkCommandBuffer createEphemeralCommand(VkCommandPool&);
        void flushCommandBuffer(VkCommandBuffer&, char*);

        void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, BufferContext*);
        void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 
        void resetCommandBuffers();
        void updateUniformBuffer(uint32_t);

        void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
        VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, uint32_t);
        void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);
        void copyBufferToImage(VkBuffer&, VkImage&, uint32_t, uint32_t);
        void generateMipmaps(VkImage&, VkFormat, int32_t, int32_t, uint32_t);

        void destroySwapChain();
        void destroyBuffer(BufferContext*);
        void destroyCommandContext();
        void destroyVertexContext();
        void destroyIndexContext();
        void destroyUniformContext();
        void destroyPipeline(Pipeline*);
        void destroyImageContext();
};

