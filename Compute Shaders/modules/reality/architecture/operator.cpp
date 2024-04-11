#include "../architect.h"

#include <cstring>

    ///////////////////////////////////
    // PIPELINE GATEWAY CONSTRUCTION //
    ///////////////////////////////////

void Architect::constructGateway() 
    {
        report(LOGGER::DEBUG, "Operator - Constructing Gateway ..");
        gateway = new Gateway();

        gateway->shaders(&logical_device)
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


    /////////////////////////
    // GATEWAY DESTRUCTION //
    /////////////////////////

void Architect::destroyGateway()
    {
        report(LOGGER::DEBUG, "Operator - Destroying Gateway ..");
        vkDestroyPipeline(logical_device, gateway->pipeline, nullptr);
        vkDestroyPipelineLayout(logical_device, gateway->pipeline_layout, nullptr);
        delete gateway;
        return;
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
    
void Architect::createCommandPool() 
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

void Architect::createCommandBuffers() 
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

VkRenderPassBeginInfo Architect::getRenderPassInfo(size_t i)
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

void Architect::recordCommandBuffers(VkCommandBuffer& command_buffer, uint32_t i) 
    {
        report(LOGGER::VLINE, "\t .. Recording Command Buffer %d ..", i);

        VkCommandBufferBeginInfo _begin_info = createBeginInfo();
        VK_TRY(vkBeginCommandBuffer(command_buffer, &_begin_info));

        VkRenderPassBeginInfo _render_pass_info = getRenderPassInfo(i);
        vkCmdBeginRenderPass(command_buffer, &_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gateway->pipeline);

        VkBuffer _vertex_buffers[] = {vertex.buffer};
        VkDeviceSize _offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, _vertex_buffers, _offsets);

        VkViewport _viewport = getViewport(swapchain.extent);
        vkCmdSetViewport(command_buffer, 0, 1, &_viewport);

        VkRect2D _scissor = getScissor(swapchain.extent);
        vkCmdSetScissor(command_buffer, 0, 1, &_scissor);

        vkCmdDraw(command_buffer, gateway->vertices.size(), 1, 0, 0);

        vkCmdEndRenderPass(command_buffer);

        VK_TRY(vkEndCommandBuffer(command_buffer));

        return;
    }

void Architect::resetCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Resetting Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_TRY(vkResetCommandBuffer(frames[i].cmd_buffer, 0));
        }

        VK_TRY(vkResetCommandBuffer(queues.cmd_buf_xfr, 0));
        VK_TRY(vkResetCommandBuffer(queues.cmd_buf_cmp, 0));

        return;
    }

    
    //////////////////////////
    // VERTEX BUFFER OBJECT //
    //////////////////////////

void Architect::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(gateway->vertices[0]) * gateway->vertices.size();
        VkBufferUsageFlags _staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags _staging_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkBufferUsageFlags _vertex_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags _vertex_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        BufferContext _staging;
        createBuffer(_buffer_size, _staging_usage, _staging_properties, _staging.buffer, _staging.memory);

        void* data;
        vkMapMemory(logical_device, _staging.memory, 0, _buffer_size, 0, &data);
        memcpy(data, gateway->vertices.data(), (size_t)_buffer_size);
        vkUnmapMemory(logical_device, _staging.memory);

        createBuffer(_buffer_size, _vertex_usage, _vertex_properties, vertex.buffer, vertex.memory);
        copyBuffer(_staging.buffer, vertex.buffer, _buffer_size);

        destroyBuffer(&_staging);

        return;
    }

void Architect::destroyVertexContext() 
    {
        report(LOGGER::VERBOSE, "Architect - Destroying Vertex Context ..");

        destroyBuffer(&vertex);

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

void Architect::createSyncObjects() 
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
