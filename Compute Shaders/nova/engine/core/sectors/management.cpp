#include "../core.h"

#include <array>

    ///////////////////////////
    // PIPELINE CONSTRUCTION //
    ///////////////////////////

void NovaCore::destroyPipeline(Pipeline* pipeline)
    {
        report(LOGGER::DEBUG, "Management - Destroying Pipeline.");
        vkDestroyPipeline(logical_device, pipeline->instance, nullptr);
        vkDestroyPipelineLayout(logical_device, pipeline->layout, nullptr);
        delete pipeline;
        return;
    }

void NovaCore::constructGraphicsPipeline()
    { 
        report(LOGGER::DEBUG, "Management - Constructing Graphics Pipeline .."); 
        
        graphics_pipeline = new Pipeline();

        graphics_pipeline->shaders(&logical_device)
                .vertexInput()
                .inputAssembly()
                .viewportState()
                .rasterizer()
                .multisampling()
                .colorBlending()
                .dynamicState()
                .createLayout(&logical_device, &descriptor.layout)
                .pipe(&render_pass)
                .create(&logical_device);

        return; 
    }
    
void NovaCore::constructComputePipeline()
    { 
        report(LOGGER::DEBUG, "Management - Constructing Compute Pipeline .."); 
        
        return; 
    }

    //////////////////////////
    // RENDER PASS CREATION //
    //////////////////////////

VkAttachmentDescription NovaCore::getColorAttachment()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment ..");

        return {
            .flags = 0,
            .format = swapchain.format,
            .samples = msaa_samples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }

VkFormat NovaCore::findDepthFormat(VkImageTiling tiling)
    {
        report(LOGGER::VLINE, "\t\t .. Finding Depth Format ..");

        const VkFormatFeatureFlags _FEATURES = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        const std::vector<VkFormat> _CANDIDATES = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        for (VkFormat _format : _CANDIDATES) 
            {
                VkFormatProperties _props;
                vkGetPhysicalDeviceFormatProperties(physical_device, _format, &_props);

                if ((tiling == VK_IMAGE_TILING_LINEAR && (_props.linearTilingFeatures & _FEATURES) == _FEATURES) ||
                    (tiling == VK_IMAGE_TILING_OPTIMAL && (_props.optimalTilingFeatures & _FEATURES) == _FEATURES)) 
                    {
                        return _format;
                    }
            }
        
        report(LOGGER::ERROR, "Management - Failed to find a suitable depth format.");
        return VK_FORMAT_UNDEFINED;
    }

void NovaCore::createColorResources()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Resources ..");

        createImage(swapchain.extent.width, swapchain.extent.height, 1, msaa_samples, swapchain.format, 
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color.image, color.memory);
        
        VkImageViewCreateInfo _view_info = createImageViewInfo(color.image, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        VK_TRY(vkCreateImageView(logical_device, &_view_info, nullptr, &color.view));

        return;
    
    }

void NovaCore::createDepthResources() 
    {
        report(LOGGER::VLINE, "\t\t .. Creating Depth Resources ..");

        VkFormat _depth_format = findDepthFormat(VK_IMAGE_TILING_OPTIMAL);
    
        createImage(swapchain.extent.width, swapchain.extent.height, 1, msaa_samples, _depth_format, 
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth.image, depth.memory);
        
        VkImageViewCreateInfo _view_info = createImageViewInfo(depth.image, _depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        VK_TRY(vkCreateImageView(logical_device, &_view_info, nullptr, &depth.view));

        return;
    }

VkAttachmentDescription NovaCore::getDepthAttachment()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Depth Attachment ..");

        return {
            .flags = 0,
            .format = findDepthFormat(VK_IMAGE_TILING_OPTIMAL),
            .samples = msaa_samples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
    }

static inline VkAttachmentReference _getColorAttachmentRef()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment Reference ..");

        return {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

static inline VkSubpassDescription _getSubpassDescription(VkAttachmentReference* color_attachment_ref)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Subpass Description");

        return {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = color_attachment_ref
        };
    }

static inline VkRenderPassCreateInfo _getRenderPassInfo(VkAttachmentDescription* color_attachment, VkSubpassDescription* subpass_description)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Render Pass Info ..");


        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = color_attachment,
            .subpassCount = 1,
            .pSubpasses = subpass_description
        };
    }

void NovaCore::createRenderPass()
    {
        report(LOGGER::VLINE, "\t .. Creating Render Pass ..");

        //log();
        VkAttachmentDescription _color_attachment = getColorAttachment();
        VkAttachmentDescription _depth_attachment = getDepthAttachment();
        VkAttachmentReference _color_attachment_ref = _getColorAttachmentRef();
        VkSubpassDescription _subpass_description = _getSubpassDescription(&_color_attachment_ref);
        VkRenderPassCreateInfo render_pass_info = _getRenderPassInfo(&_color_attachment, &_subpass_description);
        
        VK_TRY(vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass));

        return;
    }


VkRenderPassBeginInfo NovaCore::getRenderPassInfo(size_t i)
    {
        return {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = render_pass,
                .framebuffer = swapchain.framebuffers[i],
                .renderArea = {
                    .offset = {0, 0},
                    .extent = swapchain.extent
                },
                .clearValueCount = 1,
                .pClearValues = &CLEAR_COLOR
            };
    }

    ///////////////////////////
    // DESCRIPTOR SET LAYOUT //
    ///////////////////////////

static inline VkDescriptorSetLayoutBinding _getVertexLayoutBinding()
    {
        return {
                binding: 0,
                descriptorType: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                descriptorCount: 1,
                stageFlags: VK_SHADER_STAGE_VERTEX_BIT,
                pImmutableSamplers: nullptr
            };
    }

static inline VkDescriptorSetLayoutBinding _getFragmentLayoutBinding()
    {
        return {
                binding: 1,
                descriptorType: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                descriptorCount: 1,
                stageFlags: VK_SHADER_STAGE_FRAGMENT_BIT,
                pImmutableSamplers: nullptr
            };
    }

static inline VkDescriptorSetLayoutCreateInfo _getLayoutInfo(std::array<VkDescriptorSetLayoutBinding, 2>& bindings)
    {
        return {
                sType: VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                bindingCount: static_cast<uint32_t>(bindings.size()),
                pBindings: bindings.data()
            };
    }

void NovaCore::createDescriptorSetLayout() 
    {
        report(LOGGER::VLINE, "\t .. Creating Descriptor Set Layout ..");

        VkDescriptorSetLayoutBinding _ubo_layout_binding = _getVertexLayoutBinding();
        VkDescriptorSetLayoutBinding _sampler_layout_binding = _getFragmentLayoutBinding();

        std::array<VkDescriptorSetLayoutBinding, 2> _layout_binding = {_ubo_layout_binding, _sampler_layout_binding};
        VkDescriptorSetLayoutCreateInfo _layout_info = _getLayoutInfo(_layout_binding);

        VK_TRY(vkCreateDescriptorSetLayout(logical_device, &_layout_info, nullptr, &descriptor.layout));

        return;
    }

static inline VkDescriptorPoolSize _getUniformPoolSize(uint32_t ct)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pool Size of %d ..", ct);

        return {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = ct
        };
    }

static inline VkDescriptorPoolSize _getSamplerPoolSize(uint32_t ct)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pool Size of %d ..", ct);

        return {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = ct
        };
    }

static inline VkDescriptorPoolCreateInfo _getPoolInfo(uint32_t ct, std::array<VkDescriptorPoolSize, 2>& sizes)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pools Info with size %d ..", sizes.size());

        return {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = ct,
            .poolSizeCount = static_cast<uint32_t>(sizes.size()),
            .pPoolSizes = sizes.data()
        };
    }

void NovaCore::constructDescriptorPool() 
    {
        report(LOGGER::VLINE, "\t .. Constructing Descriptor Pool ..");

        std::array<VkDescriptorPoolSize, 2> _pool_size = {
            _getUniformPoolSize(MAX_FRAMES_IN_FLIGHT),
            _getSamplerPoolSize(MAX_FRAMES_IN_FLIGHT)
        };

        VkDescriptorPoolCreateInfo _pool_info = _getPoolInfo(MAX_FRAMES_IN_FLIGHT, _pool_size);

        VK_TRY(vkCreateDescriptorPool(logical_device, &_pool_info, nullptr, &descriptor.pool));

        return;
    }

static inline VkDescriptorSetAllocateInfo _getDescriptorSetAllocateInfo(uint32_t ct, VkDescriptorPool* pool, std::vector<VkDescriptorSetLayout>& layouts)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Set Allocate Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = *pool,
            .descriptorSetCount = ct,
            .pSetLayouts = layouts.data()
        };
    }

static inline VkDescriptorBufferInfo _getDescriptorBufferInfo(VkBuffer* buffer, VkDeviceSize size)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Buffer Info ..");

        return {
            .buffer = *buffer,
            .offset = 0,
            .range = size
        };
    }

static inline VkDescriptorImageInfo _getDescriptorImageInfo(ImageContext* texture)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Image Info ..");

        return {
            .sampler = texture->sampler,
            .imageView = texture->view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

static inline VkWriteDescriptorSet _getUniformDescriptorWrite(VkDescriptorSet* set, VkDescriptorBufferInfo* buffer_info)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Write ..");

        return {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = *set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = buffer_info,
            .pTexelBufferView = nullptr
        };
    }

static inline VkWriteDescriptorSet _getSamplerDescriptorWrite(VkDescriptorSet* set, VkDescriptorImageInfo* image_info)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Write ..");

        return {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = *set,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };
    }

void NovaCore::createDescriptorSets() 
    {
        report(LOGGER::VLINE, "\t .. Creating Descriptor Sets ..");

        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor.layout);
        VkDescriptorSetAllocateInfo _alloc_info = _getDescriptorSetAllocateInfo(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), &descriptor.pool, layouts);

        descriptor.sets.resize(MAX_FRAMES_IN_FLIGHT);

        VK_TRY(vkAllocateDescriptorSets(logical_device, &_alloc_info, descriptor.sets.data()));

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                VkDescriptorBufferInfo _buffer_info = _getDescriptorBufferInfo(&uniform[i].buffer, sizeof(MVP));
                VkDescriptorImageInfo _image_info = _getDescriptorImageInfo(&texture);
                
                std::array<VkWriteDescriptorSet, 2> _write_descriptor = {
                    _getUniformDescriptorWrite(&descriptor.sets[i], &_buffer_info),
                    _getSamplerDescriptorWrite(&descriptor.sets[i], &_image_info)
                };

                vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(_write_descriptor.size()), _write_descriptor.data(), 0, nullptr);
            }
    }

    /////////////////////
    // COMMAND BUFFERS //
    /////////////////////

static inline VkCommandPoolCreateInfo _createCommandPoolInfo(unsigned int queue_family_index, char* name)
    {
        report(LOGGER::VLINE, "\t\t .. Creating %s Command Pool Info ..", name);
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue_family_index
            };
    }
    
void NovaCore::createCommandPool() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Pool ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            char name[32];
            sprintf(name, "Graphics %d", i);
            VkCommandPoolCreateInfo _gfx_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.graphics_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_gfx_cmd_pool_create_info, nullptr, &frames[i].cmd.pool));
        }

        {
            char name[] = "Transfer";
            VkCommandPoolCreateInfo _xfr_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.transfer_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_xfr_cmd_pool_create_info, nullptr, &queues.xfr.pool));
        }

        {
            char name[] = "Compute";
            VkCommandPoolCreateInfo _cmp_cmd_pool_create_info = _createCommandPoolInfo(queues.indices.compute_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_cmp_cmd_pool_create_info, nullptr, &queues.cmp.pool));
        }

        return;
    }

inline VkCommandBufferAllocateInfo NovaCore::createCommandBuffersInfo(VkCommandPool& cmd_pool, char* name)
    {
        report(LOGGER::VLINE, "\t\t\t\t .. Creating %s Command Buffer Info  ..", name);

        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = cmd_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
    }

void NovaCore::createCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            char name[32];
            sprintf(name, "Graphics %d", i);
            VkCommandBufferAllocateInfo _gfx_cmd_buf_alloc_info = createCommandBuffersInfo(frames[i].cmd.pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_gfx_cmd_buf_alloc_info, &frames[i].cmd.buffer));
        }

        {
            char name[] = "Transfer";
            VkCommandBufferAllocateInfo _xfr_cmd_buf_alloc_info = createCommandBuffersInfo(queues.xfr.pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_xfr_cmd_buf_alloc_info, &queues.xfr.buffer));
        }

        {
            char name[] = "Compute";
            VkCommandBufferAllocateInfo _cmp_cmd_buf_alloc_info = createCommandBuffersInfo(queues.cmp.pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_cmp_cmd_buf_alloc_info, &queues.cmp.buffer));
        }

        return;
    }


VkCommandBuffer NovaCore::createEphemeralCommand(VkCommandPool& pool) 
    {
        report(LOGGER::VLINE, "\t\t\t .. Creating Ephemeral Command Buffer ..");

        VkCommandBuffer _buffer;

        char name[] = "Ephemeral";
        VkCommandBufferAllocateInfo _tmp_alloc_info = createCommandBuffersInfo(pool, name);

        VK_TRY(vkAllocateCommandBuffers(logical_device, &_tmp_alloc_info, &_buffer));

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(_buffer, &_begin_info));

        return _buffer;
    }

static inline VkSubmitInfo _createSubmitInfo(VkCommandBuffer* cmd)
    {
        report(LOGGER::VLINE, "\t .. Creating Submit Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = cmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr
        };
    }

void NovaCore::flushCommandBuffer(VkCommandBuffer& buf, char* name) 
    {
        report(LOGGER::VLINE, "\t .. Ending %s Command Buffer ..", name);

        VK_TRY(vkEndCommandBuffer(buf));

        // Submit the command buffer
        VkSubmitInfo _submit_info = _createSubmitInfo(&buf);
        VK_TRY(vkQueueSubmit(queues.transfer, 1, &_submit_info, VK_NULL_HANDLE));
        VK_TRY(vkQueueWaitIdle(queues.transfer));

        // Free the command buffer
        vkFreeCommandBuffers(logical_device, queues.xfr.pool, 1, &buf);

        return;
    }

void NovaCore::destroyCommandContext()
    {
        report(LOGGER::VERBOSE, "Management - Destroying Semaphores, Fences and Command Pools ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(logical_device, frames[i].image_available, nullptr);
                vkDestroySemaphore(logical_device, frames[i].render_finished, nullptr);
                //vkDestroySemaphore(logical_device, frames[i].transfer_finished, nullptr);
                //vkDestroySemaphore(logical_device, frames[i].compute_finished, nullptr);
                vkDestroyFence(logical_device, frames[i].in_flight, nullptr);
                vkDestroyCommandPool(logical_device, frames[i].cmd.pool, nullptr);
            }

        vkDestroyCommandPool(logical_device, queues.xfr.pool, nullptr);
        vkDestroyCommandPool(logical_device, queues.cmp.pool, nullptr);
    }

    /////////////////////
    // SYNC STRUCTURES //
    /////////////////////

static VkSemaphoreCreateInfo createSemaphoreInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Semaphore Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };
    }

static VkFenceCreateInfo createFenceInfo()
    {
        report(LOGGER::VLINE, "\t .. Creating Fence Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
    }

void NovaCore::createSyncObjects() 
    {
        report(LOGGER::VLINE, "\t .. Creating Sync Objects ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkSemaphoreCreateInfo semaphore_info = createSemaphoreInfo();
            VkFenceCreateInfo fence_info = createFenceInfo();

            VK_TRY(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &frames[i].image_available));
            VK_TRY(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &frames[i].render_finished));
            VK_TRY(vkCreateFence(logical_device, &fence_info, nullptr, &frames[i].in_flight));
        }

        return;
    }