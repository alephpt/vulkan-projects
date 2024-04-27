#include "../core.h"

    // Get Current Frame
FrameData& NovaCore::current_frame() { { return frames[_frame_ct % MAX_FRAMES_IN_FLIGHT]; } }


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
                report(LOGGER::DLINE, "\t\t\tCommand Pool (Graphics %d): %p", i, frames[i].cmd.pool);
                report(LOGGER::DLINE, "\t\t\tCommand Buffer (Graphics %d): %p", i, frames[i].cmd.buffer);
            }
        report(LOGGER::DLINE, "\t\tTransfer Family Index: %d", queues.indices.transfer_family.value());
        report(LOGGER::DLINE, "\t\tTransfer: %p", queues.transfer);
        report(LOGGER::DLINE, "\t\tCommand Pool (Graphics): %p", queues.gfx.pool);
        report(LOGGER::DLINE, "\t\tCommand Buffer (Graphics): %p", queues.gfx.buffer);
        report(LOGGER::DLINE, "\t\tCommand Pool (Transfer): %p", queues.xfr.pool);
        report(LOGGER::DLINE, "\t\tCommand Buffer (Transfer): %p", queues.xfr.buffer);
        report(LOGGER::DLINE, "\t\tCompute Family Index: %d", queues.indices.compute_family.value());
        report(LOGGER::DLINE, "\t\tCompute: %p", queues.compute);
        report(LOGGER::DLINE, "\t\tCommand Pool (Compute): %p", queues.cmp.pool);
        report(LOGGER::DLINE, "\t\tCommand Buffer (Compute): %p", queues.cmp.buffer);
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
                report(LOGGER::DLINE, "\t\t\tTransfer Finished: %p", frames[i].transfer_finished);
                report(LOGGER::DLINE, "\t\t\tComputer Finished: %p", frames[i].compute_finished);
                report(LOGGER::DLINE, "\t\t\tIn Flight: %p", frames[i].in_flight);
            }
    
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
