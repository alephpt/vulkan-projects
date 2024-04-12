#include "../engine.h"
#include <cstring>


    //////////////////////
    // PIPELINE GATEWAY //
    //////////////////////

void GFXEngine::constructPipeline() 
    {
        report(LOGGER::DEBUG, "Operator - Constructing Pipeline ..");
        pipeline = new Pipeline();

        pipeline->shaders(&logical_device)
                .vertexInput()
                .inputAssembly()
                .viewportState()
                .rasterizer()
                .multisampling()
                .colorBlending()
                .dynamicState()
                .layout(&logical_device)
                .pipe(&render_pass)
                .create(&logical_device);
    }

void GFXEngine::destroyPipeline()
    {
        report(LOGGER::DEBUG, "Operator - Destroying Pipeline ..");
        vkDestroyPipeline(logical_device, pipeline->pipeline, nullptr);
        vkDestroyPipelineLayout(logical_device, pipeline->pipeline_layout, nullptr);
        delete pipeline;
        return;
    }


    //////////////////////////
    // RENDER PASS CREATION //
    //////////////////////////

VkAttachmentDescription GFXEngine::colorAttachment()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment ..");

        return {
            .flags = 0,
            .format = swapchain.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }

static VkAttachmentReference colorAttachmentRef()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment Reference ..");

        return {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

static VkSubpassDescription subpassDescription(VkAttachmentReference* color_attachment_ref)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Subpass Description");

        return {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = color_attachment_ref
        };
    }

static VkRenderPassCreateInfo renderPassInfo(VkAttachmentDescription* color_attachment, VkSubpassDescription* subpass_description)
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

void GFXEngine::createRenderPass()
    {
        report(LOGGER::VLINE, "\t .. Creating Render Pass ..");

        //log();
        VkAttachmentDescription _color_attachment = colorAttachment();
        VkAttachmentReference _color_attachment_ref = colorAttachmentRef();
        VkSubpassDescription _subpass_description = subpassDescription(&_color_attachment_ref);
        VkRenderPassCreateInfo render_pass_info = renderPassInfo(&_color_attachment, &_subpass_description);
        
        VK_TRY(vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass));

        return;
    }


VkRenderPassBeginInfo GFXEngine::getRenderPassInfo(size_t i)
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


    /////////////////////
    // COMMAND BUFFERS //
    /////////////////////

static inline VkCommandPoolCreateInfo createCommandPoolInfo(unsigned int queue_family_index, char* name)
    {
        report(LOGGER::VLINE, "\t\t .. Creating %s Command Pool Info ..", name);
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue_family_index
            };
    }
    
void GFXEngine::createCommandPool() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Pool ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            char name[32];
            sprintf(name, "Graphics %d", i);
            VkCommandPoolCreateInfo _gfx_cmd_pool_create_info = createCommandPoolInfo(queues.indices.graphics_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_gfx_cmd_pool_create_info, nullptr, &frames[i].cmd_pool));
        }

        {
            char name[] = "Transfer";
            VkCommandPoolCreateInfo _xfr_cmd_pool_create_info = createCommandPoolInfo(queues.indices.transfer_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_xfr_cmd_pool_create_info, nullptr, &queues.cmd_pool_xfr));
        }

        {
            char name[] = "Compute";
            VkCommandPoolCreateInfo _cmp_cmd_pool_create_info = createCommandPoolInfo(queues.indices.compute_family.value(), name);
            VK_TRY(vkCreateCommandPool(logical_device, &_cmp_cmd_pool_create_info, nullptr, &queues.cmd_pool_cmp));
        }

        return;
    }

static inline VkCommandBufferAllocateInfo createCommandBuffersInfo(VkCommandPool& cmd_pool, char* name)
    {
        report(LOGGER::VLINE, "\t\t .. Creating %s Command Buffer Info  ..", name);

        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = cmd_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
    }

void GFXEngine::createCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            char name[32];
            sprintf(name, "Graphics %d", i);
            VkCommandBufferAllocateInfo _gfx_cmd_buf_alloc_info = createCommandBuffersInfo(frames[i].cmd_pool, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_gfx_cmd_buf_alloc_info, &frames[i].cmd_buffer));
        }

        {
            char name[] = "Transfer";
            VkCommandBufferAllocateInfo _xfr_cmd_buf_alloc_info = createCommandBuffersInfo(queues.cmd_pool_xfr, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_xfr_cmd_buf_alloc_info, &queues.cmd_buf_xfr));
        }

        {
            char name[] = "Compute";
            VkCommandBufferAllocateInfo _cmp_cmd_buf_alloc_info = createCommandBuffersInfo(queues.cmd_pool_cmp, name);
            VK_TRY(vkAllocateCommandBuffers(logical_device, &_cmp_cmd_buf_alloc_info, &queues.cmd_buf_cmp));
        }

        return;
    }

static inline VkViewport getViewport(VkExtent2D extent)
    {
        return {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
    }

static inline VkRect2D getScissor(VkExtent2D extent)
    {
        return {
            .offset = {0, 0},
            .extent = extent
        };
    }

void GFXEngine::recordCommandBuffers(VkCommandBuffer& command_buffer, uint32_t i) 
    {
        report(LOGGER::VLINE, "\t .. Recording Command Buffer %d ..", i);

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(command_buffer, &_begin_info));

        VkRenderPassBeginInfo _render_pass_info = getRenderPassInfo(i);
        vkCmdBeginRenderPass(command_buffer, &_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        VkViewport _viewport = getViewport(swapchain.extent);
        vkCmdSetViewport(command_buffer, 0, 1, &_viewport);

        VkRect2D _scissor = getScissor(swapchain.extent);
        vkCmdSetScissor(command_buffer, 0, 1, &_scissor);

        VkBuffer _vertex_buffers[] = {vertex.buffer};
        VkDeviceSize _offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, _vertex_buffers, _offsets);
        vkCmdBindIndexBuffer(command_buffer, index.buffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(pipeline->indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(command_buffer);

        VK_TRY(vkEndCommandBuffer(command_buffer));

        return;
    }

void GFXEngine::resetCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Resetting Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_TRY(vkResetCommandBuffer(frames[i].cmd_buffer, 0));
        }

        VK_TRY(vkResetCommandBuffer(queues.cmd_buf_xfr, 0));
        VK_TRY(vkResetCommandBuffer(queues.cmd_buf_cmp, 0));

        return;
    }

    
    ////////////////////////////
    // OBJECT BUFFER CREATION //
    ////////////////////////////

static const VkBufferUsageFlags _staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
static const VkMemoryPropertyFlags _staging_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

static const VkBufferUsageFlags _index_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
static const VkMemoryPropertyFlags _index_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
static const VkBufferUsageFlags _vertex_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
static const VkMemoryPropertyFlags _vertex_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

void GFXEngine::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(pipeline->vertices[0]) * pipeline->vertices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, _staging.buffer, _staging.memory);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, pipeline->vertices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _vertex_usage, _vertex_properties, vertex.buffer, vertex.memory);
        copyBuffer(_staging.buffer, vertex.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void GFXEngine::constructIndexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Index Buffer ..");

        VkDeviceSize _buffer_size = sizeof(pipeline->indices[0]) * pipeline->indices.size();

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, _staging.buffer, _staging.memory);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, pipeline->indices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _index_usage, _index_properties, index.buffer, index.memory);
        copyBuffer(_staging.buffer, index.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void GFXEngine::destroyVertexContext() 
    {
        report(LOGGER::VERBOSE, "GFXEngine - Destroying Vertex Context ..");

        destroyBuffer(&vertex);

        return;
    }


void GFXEngine::destroyIndexContext() 
    {
        report(LOGGER::VERBOSE, "GFXEngine - Destroying Vertex Context ..");

        destroyBuffer(&index);

        return;
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

void GFXEngine::createSyncObjects() 
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
