#pragma once
#include "./sectors/00atomic/pipeline/pipeline.h"
#include "./components/utility/lexicon.h"


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
        ComputeData compute[MAX_COMPUTE_QUEUES]; // TODO: Get Max Compute Queues from Device when we query the queue count
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
        const std::array<VkClearValue, 2> CLEAR_VALUES = {
            CLEAR_COLOR,
            {1.0f, 0}
        };
        FrameData& current_frame();
        int _frame_ct = 0;
        VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;
        uint32_t mip_lvls = 1;

        void logQueues();
        void logSwapChain();
        void logFrameData();
        void logComputeData();
        void logTransferData();

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
        void recreateSwapChain();

        VkFormat findDepthFormat(VkImageTiling);
        VkAttachmentDescription getDepthAttachment();
        VkRenderPassBeginInfo getRenderPassInfo(size_t);
        VkAttachmentDescription getColorAttachment();

        VkCommandBufferBeginInfo createBeginInfo();
        VkCommandBufferAllocateInfo createCommandBuffersInfo(VkCommandPool&, char*);
        VkCommandBuffer createEphemeralCommand(VkCommandPool&);
        void flushCommandBuffer(VkCommandBuffer&, char*, VkQueue&, VkCommandPool&);

        void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, BufferContext*);
        void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize, VkQueue&, VkCommandPool&);
        void recordCommandBuffers(VkCommandBuffer&, uint32_t); 
        void resetCommandBuffers();
        void updateUniformBuffer(uint32_t);

        VkImageMemoryBarrier getMemoryBarrier(VkImage&, VkImageLayout&, VkImageLayout&, uint32_t mip_level = 1);
        void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
        VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, uint32_t);
        void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout, VkQueue&, VkCommandPool&, uint32_t);
        void copyBufferToImage(VkBuffer&, VkImage&, uint32_t, uint32_t, VkQueue&, VkCommandPool&);
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


const VkMemoryPropertyFlagBits _MEMORY_DEVICE_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
const VkImageUsageFlags _COLOR_ATTACHMENT_BIT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
const VkBufferUsageFlags _TRANSFER_SRC_BIT = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
const VkBufferUsageFlags _INDEX_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
const VkBufferUsageFlags _VERTEX_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
const VkBufferUsageFlags _IMAGE_BUFFER_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
const VkBufferUsageFlags _IMAGE_TRANSFER_BIT = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
const VkMemoryPropertyFlags _STAGING_PROPERTIES_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
const VkMemoryPropertyFlags _LOCAL_DEVICE_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
const VkImageLayout _IMAGE_LAYOUT_DST = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
const VkImageLayout _IMAGE_LAYOUT_SRC = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; 
const VkAccessFlags _TRANSFER_WRITE_BIT = VK_ACCESS_TRANSFER_WRITE_BIT;
const VkAccessFlags _TRANSFER_READ_BIT = VK_ACCESS_TRANSFER_READ_BIT;
const VkAccessFlags _SHADER_READ_BIT = VK_ACCESS_SHADER_READ_BIT;
const VkImageLayout _IMAGE_LAYOUT_UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED;
const VkImageLayout _IMAGE_LAYOUT_READ_ONLY = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
const VkPipelineStageFlagBits _PIPELINE_TRANSFER_BIT = VK_PIPELINE_STAGE_TRANSFER_BIT;
const VkPipelineStageFlagBits _PIPELINE_FRAGMENT_BIT = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
const VkFormat _SRGB_FORMAT_888 = VK_FORMAT_R8G8B8A8_SRGB;
const VkImageAspectFlagBits _IMAGE_COLOR_BIT = VK_IMAGE_ASPECT_COLOR_BIT;
