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

static VkCommandPoolCreateInfo createCommandPoolInfo(unsigned int queue_family_index)
    {
        report(LOGGER::VLINE, "\t .. Creating Command Pool Info ..");
        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue_family_index
            };
    }
    

VkCommandBufferAllocateInfo Architect::createCommandBuffers(unsigned int n)
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffer %d ..", n);

        return {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = frames[n].command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
    }

void Architect::createCommandPool() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Pool ..");

        VkCommandPoolCreateInfo _create_info = createCommandPoolInfo(queues.indices.graphics_family.value());

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            { VK_TRY(vkCreateCommandPool(logical_device, &_create_info, nullptr, &frames[i].command_pool)); }

        return;
    }

void Architect::createCommandBuffers() 
    {
        report(LOGGER::VLINE, "\t .. Creating Command Buffers ..");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkCommandBufferAllocateInfo _alloc_info = createCommandBuffers(i);
                VK_TRY(vkAllocateCommandBuffers(logical_device, &_alloc_info, &frames[i].command_buffer));
            }

        return;
    }

static VkCommandBufferBeginInfo createBeginInfo() {
    return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
        };
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

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_TRY(vkResetCommandBuffer(frames[i].command_buffer, 0));
            }

        return;
    }

    
    //////////////////////////
    // VERTEX BUFFER OBJECT //
    //////////////////////////

void Architect::constructVertexBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Buffer ..");

        VkDeviceSize _buffer_size = sizeof(gateway->vertices[0]) * gateway->vertices.size();
        VkBufferUsageFlags _usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags _properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        createBuffer(_buffer_size, _usage, _properties, vertex.buffer, vertex.memory);

        void* data;
        vkMapMemory(logical_device, vertex.memory, 0, _buffer_size, 0, &data);
        std::memcpy(data, gateway->vertices.data(), (size_t) _buffer_size);
        vkUnmapMemory(logical_device, vertex.memory);

        return;
    }

void Architect::destroyVertexContext() 
    {
        report(LOGGER::VERBOSE, "Architect - Destroying Vertex Context ..");

        vkDestroyBuffer(logical_device, vertex.buffer, nullptr);
        vkFreeMemory(logical_device, vertex.memory, nullptr);

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
