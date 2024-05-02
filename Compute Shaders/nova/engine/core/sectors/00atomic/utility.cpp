#include "../../core.h"


    // Get Current Frame
FrameData& NovaCore::current_frame() { { return frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }
ComputeData& NovaCore::current_compute() { { return computes[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }



    ///////////////////////
    // Memory Allocation //
    ///////////////////////

static inline uint32_t findMemoryType(VkPhysicalDevice& physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        report(LOGGER::VLINE, "\t\t\t\t .. Finding Memory Type ..");

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
            { if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
                    { return i; } }

        VK_TRY(VK_ERROR_INITIALIZATION_FAILED);
        return -1;
    }

VkMemoryAllocateInfo NovaCore::getMemoryAllocateInfo(VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags properties)
    {
        report(LOGGER::VLINE, "\t\t\t .. Creating Memory Allocate Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = mem_reqs.size,
            .memoryTypeIndex = findMemoryType(physical_device, mem_reqs.memoryTypeBits, properties)
        };
    }






    /////////////
    // LOGGING //
    /////////////

void NovaCore::logQueues() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Queues ..");
        report(LOGGER::DLINE, "\t\tFamilies: %d", queues.families.size());
        report(LOGGER::DLINE, "\t\tPresent Family Index: %d", queues.indices.present_family.value());
        report(LOGGER::DLINE, "\t\tPresent: %p", queues.present);
        report(LOGGER::DLINE, "\t\tGraphics Family Index: %d", queues.indices.graphics_family.value());
        report(LOGGER::DLINE, "\t\tGraphics: %p", queues.graphics);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                report(LOGGER::DLINE, "\t\t\tCommand Buffer (Graphics %d): %p", i, frames[i].command_buffer);
            }
        report(LOGGER::DLINE, "\t\t\tGraphics Command Pool: %p", queues.command_pool);
        
        report(LOGGER::DLINE, "\t\tTransfer Family Index: %d", queues.indices.transfer_family.value());
        report(LOGGER::DLINE, "\t\tCommand Pool (Transfer): %p", queues.transfer.pool);

        report(LOGGER::DLINE, "\t\tCompute Family Index: %d", queues.indices.compute_family.value());
        report(LOGGER::DLINE, "\t\tCommand Pool (Compute): %p", queues.compute.pool);
        for (size_t i = 0; i < MAX_COMPUTE_QUEUES; i++) 
            {
                report(LOGGER::DLINE, "\t\t\tCommand Buffer (Compute %d): %p", i, computes[i].command_buffer);
            }


        report(LOGGER::DLINE, "\t\tPriorities: %d", queues.priorities.size());
    }

void NovaCore::logFrameData()
    {
        report(LOGGER::DEBUG, "\t .. Logging Frame Data ..");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                report(LOGGER::DLINE, "\t\tFrame %d", i);
                report(LOGGER::DLINE, "\t\t\tImage Available: %p", frames[i].image_available);
                report(LOGGER::DLINE, "\t\t\tRender Finished: %p", frames[i].render_finished);
                report(LOGGER::DLINE, "\t\t\tIn Flight: %p", frames[i].in_flight);
            }
    }

void NovaCore::logComputeData()
    {
        report(LOGGER::DEBUG, "\t .. Logging Compute Data ..");
        for (size_t i = 0; i < MAX_COMPUTE_QUEUES; i++) 
            {
                report(LOGGER::DLINE, "\t\tCompute %d", i);
                report(LOGGER::DLINE, "\t\t\tCompute Finished: %p", computes[i].finished);
                report(LOGGER::DLINE, "\t\t\tIn Flight: %p", computes[i].in_flight);
            }
    }

void NovaCore::logTransferData()
    {
        report(LOGGER::DEBUG, "\t .. Logging Transfer Data ..");
        report(LOGGER::DLINE, "\t\tTransfer Finished: %p", queues.transfer.transfer_finished);
        report(LOGGER::DLINE, "\t\tIn Flight: %p", queues.transfer.in_flight);
    }

void NovaCore::logSwapChain() 
    {
        report(LOGGER::DEBUG, "\t .. Logging SwapChain ..");
        report(LOGGER::DLINE, "\t\tSwapchain: %p", swapchain.instance);
        report(LOGGER::DLINE, "\t\tImage Count: %d", swapchain.images.size());
        report(LOGGER::DLINE, "\t\tImage Views: %d", swapchain.image_views.size());
        report(LOGGER::DLINE, "\t\tFramebuffers: %d", swapchain.framebuffers.size());
        report(LOGGER::DLINE, "\t\tSupport: %d Formats", swapchain.support.formats.size());
        report(LOGGER::DLINE, "\t\tSupport: %d Present Modes", swapchain.support.present_modes.size());
        report(LOGGER::DLINE, "\t\tDetails: %d Formats", swapchain.details.surface.format);
        report(LOGGER::DLINE, "\t\tDetails: %d Present Modes", swapchain.details.present_mode);
        report(LOGGER::DLINE, "\t\tDetails: %d x %d Extent", swapchain.details.extent.width, swapchain.details.extent.height);
    }

void NovaCore::log() 
    {
        report(LOGGER::DEBUG, "\t .. Logging Context ..");
        report(LOGGER::DLINE, "\t\tInstance: %p", instance);
        report(LOGGER::DLINE, "\t\tPhysical Device: %p", physical_device);
        report(LOGGER::DLINE, "\t\tLogical Device: %p", logical_device);
        report(LOGGER::DLINE, "\t\tSurface: %p", surface);
        logQueues();
        logSwapChain();
        report(LOGGER::DLINE, "\t\tRender Pass: %p", render_pass);
        report(LOGGER::DLINE, "\t\tPresent Info: %p", &present);
        logFrameData();
    }
