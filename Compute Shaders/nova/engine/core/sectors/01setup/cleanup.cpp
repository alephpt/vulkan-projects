#include "../../core.h"

NovaCore::~NovaCore() 
    {
        report(LOGGER::INFO, "NovaCore - Destroying Context ..");

        destroySwapChain();
        queues.deletion.flush();
        vkDestroyDescriptorPool(logical_device, descriptor.pool, nullptr);
        vkDestroyDescriptorSetLayout(logical_device, descriptor.layout, nullptr);
        destroyVertexContext();
        destroyIndexContext();
        destroyCommandContext();
        destroyPipeline(graphics_pipeline);
        destroyPipeline(compute_pipeline);
        destroyComputeResources();

        report(LOGGER::VLINE, "\t .. Destroying Pipeline and Render Pass.");
        vkDestroyRenderPass(logical_device, render_pass, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Logical Device.");
        vkDestroyDevice(logical_device, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Surface.");
        vkDestroySurfaceKHR(instance, surface, nullptr);

        report(LOGGER::VLINE, "\t .. Destroying Instance.");
        vkDestroyInstance(instance, nullptr);

        _blankContext();
    }

void NovaCore::destroySwapChain() 
    {
        report(LOGGER::VERBOSE, "Presentation - Destroying Swapchain ..");

        for (const auto& _frame_buffers : swapchain.framebuffers) 
            { vkDestroyFramebuffer(logical_device, _frame_buffers, nullptr); }
        
        //swapchain.framebuffers.clear();

        
        for (const auto& _image_view : swapchain.image_views) 
            { vkDestroyImageView(logical_device, _image_view, nullptr); }

        //swapchain.image_views.clear();

        vkDestroySwapchainKHR(logical_device, swapchain.instance, nullptr);

        return;
    }

void NovaCore::destroyVertexContext() 
    { 
        if (vertex.buffer != VK_NULL_HANDLE) 
            { 
                report(LOGGER::VERBOSE, "Scene - Destroying Vertex Context .."); 
                destroyBuffer(&vertex); 
            }

        return; 
    }

void NovaCore::destroyIndexContext() 
    { 
        if (index.buffer != VK_NULL_HANDLE) 
            { 
                report(LOGGER::VERBOSE, "Scene - Destroying Index Context .."); 
                destroyBuffer(&index); 
            }

        return; 
    }

void NovaCore::destroyCommandContext()
    {
        report(LOGGER::VERBOSE, "Management - Destroying Semaphores, Fences and Command Pools ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(logical_device, frames[i].image_available, nullptr);
                vkDestroySemaphore(logical_device, frames[i].render_finished, nullptr);
                vkDestroyFence(logical_device, frames[i].in_flight, nullptr);
            }

        vkDestroyCommandPool(logical_device, queues.command_pool, nullptr);
        vkDestroyCommandPool(logical_device, queues.transfer.pool, nullptr);
    }

void NovaCore::destroyPipeline(GraphicsPipeline* pipeline)
    {
        report(LOGGER::DEBUG, "Management - Destroying Pipeline.");
        vkDestroyPipeline(logical_device, pipeline->instance, nullptr);
        vkDestroyPipelineLayout(logical_device, pipeline->layout, nullptr);
        delete pipeline;
        return;
    }

void NovaCore::destroyPipeline(ComputePipeline* pipeline)
    {
        report(LOGGER::DEBUG, "Management - Destroying Pipeline.");
        vkDestroyPipeline(logical_device, pipeline->instance, nullptr);
        vkDestroyPipelineLayout(logical_device, pipeline->layout, nullptr);
        delete pipeline;
        return;
    }

void NovaCore::destroyBuffer(BufferContext* buffer) 
    {
        if (buffer->buffer != VK_NULL_HANDLE) 
            { 
                report(LOGGER::VERBOSE, "Management - Destroying Buffer ..");
                vkDestroyBuffer(logical_device, buffer->buffer, nullptr);
            }

        if (buffer->memory != VK_NULL_HANDLE) 
            { 
                report(LOGGER::VERBOSE, "Management - Freeing Buffer Memory ..");
                vkFreeMemory(logical_device, buffer->memory, nullptr); 
            }

        return;
    }

void NovaCore::destroyComputeResources()
    {
        report(LOGGER::VERBOSE, "Management - Destroying Compute Resources ..");

        // destroy compute semaphores and fences
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                vkDestroySemaphore(logical_device, computes[i].finished, nullptr);
                vkDestroyFence(logical_device, computes[i].in_flight, nullptr);
            }

        // destroy compute command pool
        vkDestroyCommandPool(logical_device, queues.compute.pool, nullptr);

        // destroy storage buffers
        for (auto& _buffer : storage) 
            { destroyBuffer(&_buffer); }

        // destroy compute descriptor set layout
        vkDestroyDescriptorSetLayout(logical_device, compute_descriptor.layout, nullptr);

    }