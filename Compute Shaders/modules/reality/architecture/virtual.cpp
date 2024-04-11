#include "../architect.h"
#include <set>

    ///////////////////////////////
    //  Virtual Swapchain Layers //
    ///////////////////////////////

SwapChainSupportDetails Architect::querySwapChainSupport(VkPhysicalDevice device)
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Support ..");
        
        SwapChainSupportDetails details = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t _format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, nullptr);

        if (_format_count != 0) 
            {
                details.formats.resize(_format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &_format_count, details.formats.data());
            }

        uint32_t _present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, nullptr);

        if (_present_mode_count != 0) 
            {
                details.present_modes.resize(_present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &_present_mode_count, details.present_modes.data());
            }

        details.capabilities.minImageExtent.width = details.capabilities.minImageExtent.width - 10;
        details.capabilities.minImageExtent.height = details.capabilities.minImageExtent.height - 10;
        details.capabilities.maxImageExtent.width = details.capabilities.maxImageExtent.width + 10;
        details.capabilities.maxImageExtent.height = details.capabilities.maxImageExtent.height + 10;

        return details;
    }



static void selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats, VkSurfaceFormatKHR* surface_format)
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Surface Format ..");

        for (const auto& available_format : available_formats) 
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
                    { *surface_format = available_format; }
            }

        *surface_format = available_formats.front();

        return;
    }


static void selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes, VkPresentModeKHR* present_mode)
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Present Mode ..");

        for (const auto& available_present_mode : available_present_modes) 
            {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
                    { *present_mode = available_present_mode; }
            }

        *present_mode = VK_PRESENT_MODE_FIFO_KHR;

        return;
    }


static void selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D* window_extent) 
    {
        report(LOGGER::VLINE, "\t .. Selecting Swap Extent ..");

        if (capabilities.currentExtent.width != UINT32_MAX) 
            { 
                window_extent->width = capabilities.currentExtent.width;
                window_extent->height = capabilities.currentExtent.height;
            }
        else 
            {
                uint32_t width = window_extent->width;
                uint32_t height = window_extent->height;

                window_extent->width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
                window_extent->height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));
            }

        return;
    }


void Architect::querySwapChainDetails()
    {
        report(LOGGER::VLINE, "\t .. Querying SwapChain Details ..");

        if (swapchain.support.formats.empty() || swapchain.support.present_modes.empty()) 
            { report(LOGGER::ERROR, "Vulkan: SwapChain support is not available."); }

        selectSwapSurfaceFormat(swapchain.support.formats, &swapchain.details.surface_format);
        selectSwapPresentMode(swapchain.support.present_modes, &swapchain.details.present_mode);
        selectSwapExtent(swapchain.support.capabilities, &swapchain.details.extent);

        return;
    }

void Architect::createSwapchainInfoKHR(VkSwapchainCreateInfoKHR* create_info, uint32_t image_count) 
    {
    *create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = swapchain.details.surface_format.format,
        .imageColorSpace = swapchain.details.surface_format.colorSpace,
        .imageExtent = swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = swapchain.support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchain.details.present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
}

void Architect::constructSwapChain() 
    {
        report(LOGGER::VLINE, "\t .. Constructing SwapChain ..");

        uint32_t _image_count = swapchain.support.capabilities.minImageCount + 1;

        if (swapchain.support.capabilities.maxImageCount > 0 && _image_count > swapchain.support.capabilities.maxImageCount) 
            { _image_count = swapchain.support.capabilities.maxImageCount; }

        //log();
        VkSwapchainCreateInfoKHR _create_info = {}; // TODO: This could be 1 line
        createSwapchainInfoKHR(&_create_info, _image_count);

        std::set<uint32_t> _unique_queue_families = { queues.indices.graphics_family.value(), queues.indices.present_family.value(), queues.indices.transfer_family.value(), queues.indices.compute_family.value() };
        std::vector<uint32_t> _queue_families(_unique_queue_families.begin(), _unique_queue_families.end());

        if (_queue_families.size() > 1) 
            {
                report(LOGGER::VLINE, "\t .. Multiple Queue Families Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                _create_info.queueFamilyIndexCount = static_cast<uint32_t>(_unique_queue_families.size());
                _create_info.pQueueFamilyIndices = _queue_families.data();
            }
        else
            {
                report(LOGGER::VLINE, "\t .. Single Queue Family Achieved ..");

                _create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                _create_info.queueFamilyIndexCount = 0;
                _create_info.pQueueFamilyIndices =  nullptr;
            }

        VkSwapchainPresentScalingCreateInfoEXT _scalability_info = {};
        if (framebuffer_resized) 
            {
                report(LOGGER::VERBOSE, "\t .. Framebuffer Resized ..");
                _scalability_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT;
                _scalability_info.scalingBehavior = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
                _create_info.pNext = &_scalability_info;
            }

        VK_TRY(vkCreateSwapchainKHR(logical_device, &_create_info, nullptr, &swapchain.instance));

        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, nullptr);
        swapchain.images.resize(_image_count);
        vkGetSwapchainImagesKHR(logical_device, swapchain.instance, &_image_count, swapchain.images.data());

        swapchain.format = swapchain.details.surface_format.format;
        swapchain.extent = swapchain.details.extent;

        report(LOGGER::VLINE, "\t .. SwapChain Constructed ..");

        return;
    }

VkImageViewCreateInfo Architect::createImageViewInfo(size_t image) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchain.images[image],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
}


void Architect::constructImageViews()
    {
        report(LOGGER::VLINE, "\t .. Constructing Image Views ..");

        swapchain.image_views.resize(swapchain.images.size());

        for (size_t i = 0; i < swapchain.images.size(); i++) 
            {
                VkImageViewCreateInfo _create_info = createImageViewInfo(i);
                VK_TRY(vkCreateImageView(logical_device, &_create_info, nullptr, &swapchain.image_views[i]));
            }

        report(LOGGER::VLINE, "\t .. Image Views Constructed ..");

        return;
    }

    ///////////////////////////
    // FRAME BUFFER CREATION //
    ///////////////////////////

void Architect::createFrameBuffers()
    {
        report(LOGGER::VLINE, "Architect - Creating Frame Buffers ..");

        swapchain.framebuffers.resize(swapchain.image_views.size());

        for (size_t i = 0; i < swapchain.image_views.size(); i++) 
            {
                VkImageView _attachments[] = { swapchain.image_views[i] };

                VkFramebufferCreateInfo _create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = 1,
                    .pAttachments = _attachments,
                    .width = swapchain.extent.width,
                    .height = swapchain.extent.height,
                    .layers = 1
                };

                VK_TRY(vkCreateFramebuffer(logical_device, &_create_info, nullptr, &swapchain.framebuffers[i]));
            }

        return;
    }

void Architect::destroySwapChain() 
    {
        report(LOGGER::VERBOSE, "Architect - Destroying Swapchain ..");

        for (const auto& _frame_buffers : swapchain.framebuffers) 
            { vkDestroyFramebuffer(logical_device, _frame_buffers, nullptr); }
        
        swapchain.framebuffers.clear();

        
        for (const auto& _image_view : swapchain.image_views) 
            { vkDestroyImageView(logical_device, _image_view, nullptr); }

        swapchain.image_views.clear();

        vkDestroySwapchainKHR(logical_device, swapchain.instance, nullptr);

        return;
    }

void Architect::recreateSwapChain() 
    {
        report(LOGGER::VERBOSE, "Architect - Recreating Swapchain ..");

        vkDeviceWaitIdle(logical_device);
        destroySwapChain();

        // TODO: Try chasing this down without the validation layer bugs being blocked 
        // to see if OUTOFDATE_KHR is the issue or if it is SUBOPTIMAL_KHR
        if (framebuffer_resized) {
            swapchain.support = querySwapChainSupport(physical_device);
            querySwapChainDetails();
        }

        constructSwapChain();
        constructImageViews();
        createFrameBuffers();
        return;
    }


    ////////////////////
    // BUFFER OBJECTS //
    ////////////////////

static inline uint32_t findMemoryType(VkPhysicalDevice& physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        report(LOGGER::VLINE, "\t .. Finding Memory Type ..");

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
            { if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
                    { return i; } }

        VK_TRY(VK_ERROR_INITIALIZATION_FAILED);
        return -1;
    }

static inline VkMemoryAllocateInfo getMemoryAllocateInfo(VkPhysicalDevice& physical_device, VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags properties)
    {
        report(LOGGER::VLINE, "\t .. Creating Memory Allocate Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = mem_reqs.size,
            .memoryTypeIndex = findMemoryType(physical_device, mem_reqs.memoryTypeBits, properties)
        };
    }

static inline VkBufferCreateInfo getBufferInfo(VkDeviceSize size, VkBufferUsageFlags usage)
    {
        report(LOGGER::VLINE, "\t .. Creating Buffer Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
        };
    }

void Architect::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
    {
        report(LOGGER::VLINE, "\t .. Creating Buffer ..");

        VkBufferCreateInfo _buffer_info = getBufferInfo(size, usage);
        VK_TRY(vkCreateBuffer(logical_device, &_buffer_info, nullptr, &buffer));

        VkMemoryRequirements _mem_reqs;
        vkGetBufferMemoryRequirements(logical_device, buffer, &_mem_reqs);

        VkMemoryAllocateInfo _alloc_info = getMemoryAllocateInfo(physical_device, _mem_reqs, properties);
        VK_TRY(vkAllocateMemory(logical_device, &_alloc_info, nullptr, &memory));

        vkBindBufferMemory(logical_device, buffer, memory, 0);

        return;
    }

static inline VkCommandBufferAllocateInfo getCommandBuffersInfo(VkCommandPool& cmd_pool, uint32_t count)
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffers Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count
        };
    }


VkCommandBufferBeginInfo Architect::createBeginInfo()
    {
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr
            };
    }

static inline VkBufferCopy getBufferCopy(VkDeviceSize size)
    {
        report(LOGGER::VLINE, "\t .. Creating Buffer Copy ..");

        return {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
    }

// TODO: We can combine the 2 static types into a more generic wrapper maybe? 
//       Or is this better?
static inline VkSubmitInfo getSubmitInfo(VkCommandBuffer* command_buffer) 
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = command_buffer,
        };
    }

// Copy data from one buffer to another using the Transfer Queue (if available)
// Asynchronous copy operations are possible by using the Transfer Queue for copying data to the GPU
// and the Compute Queue for running compute shaders, while the Graphics Queue is used for rendering
// and the Present Queue is used for presenting the swapchain images to the screen
void Architect::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo _cmd_buf_info = getCommandBuffersInfo(queues.cmd_pool_xfr, 1);
        VkCommandBuffer _cmd_buffer;
        VK_TRY(vkAllocateCommandBuffers(logical_device, &_cmd_buf_info, &_cmd_buffer));
        
        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(_cmd_buffer, &_begin_info));

        VkBufferCopy _copy_region = getBufferCopy(size);
        vkCmdCopyBuffer(_cmd_buffer, src_buffer, dst_buffer, 1, &_copy_region);

        VK_TRY(vkEndCommandBuffer(_cmd_buffer));

        VkSubmitInfo _submit_info = getSubmitInfo(&_cmd_buffer);
        VK_TRY(vkQueueSubmit(queues.graphics, 1, &_submit_info, VK_NULL_HANDLE));
        VK_TRY(vkQueueWaitIdle(queues.graphics));

        vkFreeCommandBuffers(logical_device, queues.cmd_pool_xfr, 1, &_cmd_buffer);
    }

void Architect::destroyBuffer(BufferContext* buffer) 
    {
        report(LOGGER::VLINE, "\t .. Destroying Buffer ..");

        vkDestroyBuffer(logical_device, buffer->buffer, nullptr);
        vkFreeMemory(logical_device, buffer->memory, nullptr);

        return;
    }