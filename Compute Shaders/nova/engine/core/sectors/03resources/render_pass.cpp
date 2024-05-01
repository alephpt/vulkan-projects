#include "../../core.h"


    //////////////////////////
    // RENDER PASS CREATION //
    //////////////////////////

VkAttachmentDescription NovaCore::getColorAttachment()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment ..");

        return {
            .flags = 0,
            .format = swapchain.details.surface.format,
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
        report(LOGGER::VLINE, "\t .. Creating Color Resources ..");
        
        createImage(swapchain.details.extent.width, swapchain.details.extent.height, 1, msaa_samples, swapchain.details.surface.format, 
                    VK_IMAGE_TILING_OPTIMAL, _COLOR_ATTACHMENT_BIT, _MEMORY_DEVICE_BIT, color.image, color.memory);
        
        color.view = createImageView(color.image, swapchain.details.surface.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        return;
    }

void NovaCore::createDepthResources() 
    {
        report(LOGGER::VLINE, "\t\t .. Creating Depth Resources ..");

        VkFormat _depth_format = findDepthFormat(VK_IMAGE_TILING_OPTIMAL);
    
        createImage(swapchain.details.extent.width, swapchain.details.extent.height, 1, msaa_samples, _depth_format, 
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                    _MEMORY_DEVICE_BIT, depth.image, depth.memory);
        
        depth.view = createImageView(depth.image, _depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

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

static inline VkAttachmentDescription _getColorAttachmentResolve(VkFormat format)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment Resolve ..");

        return {
            .flags = 0,
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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

static inline VkAttachmentReference _getDepthAttachmentRef()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Depth Attachment Reference ..");

        return {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
    }

static inline VkAttachmentReference _getColorAttachmentResolveRef()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Color Attachment Resolve Reference ..");

        return {
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

static inline VkSubpassDescription _getSubpassDescription(VkAttachmentReference* color_attachment_ref, 
                                                            VkAttachmentReference* depth_attachment_ref, 
                                                            VkAttachmentReference* color_attachment_resolve_ref)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Subpass Description");

        return {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = color_attachment_ref,
            .pResolveAttachments = color_attachment_resolve_ref,
            .pDepthStencilAttachment = depth_attachment_ref,
        };
    }

static inline VkSubpassDependency _getDependency()
    {
        report(LOGGER::VLINE, "\t\t .. Creating Subpass Dependency ..");

        return {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // This is used for vertex & image | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // This is used for vertex & image | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // This is used for mipmaping | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
        };
    }

static inline VkRenderPassCreateInfo _getRenderPassInfo(std::vector<VkAttachmentDescription>* attachments, VkSubpassDescription* subpass_description, VkSubpassDependency* dependency)
    {
        report(LOGGER::VLINE, "\t\t .. Getting Render Pass Create Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments->size()),
            .pAttachments = attachments->data(),
            .subpassCount = 1,
            .pSubpasses = subpass_description
        };
    }

void NovaCore::createRenderPass()
    {
        report(LOGGER::VLINE, "\t .. Creating Render Pass ..");

        //log();
        VkAttachmentDescription _color_attachment = getColorAttachment();
        //VkAttachmentDescription _depth_attachment = getDepthAttachment();
        //VkAttachmentDescription _color_resolve = _getColorAttachmentResolve(swapchain.details.surface.format);
        VkAttachmentReference _color_attachment_ref = _getColorAttachmentRef();
        //VkAttachmentReference _depth_attachment_ref = _getDepthAttachmentRef();
        //VkAttachmentReference _color_attachment_resolve_ref = _getColorAttachmentResolveRef();
        //VkSubpassDescription _subpass_description = _getSubpassDescription(&_color_attachment_ref, &_depth_attachment_ref, &_color_attachment_resolve_ref);
        VkSubpassDescription _subpass_description = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &_color_attachment_ref,
            .pDepthStencilAttachment = nullptr
        };
        VkSubpassDependency _dependency = _getDependency();

        //std::array<VkAttachmentDescription, 3> _attachments = {_color_attachment, _depth_attachment, _color_resolve};
        std::vector<VkAttachmentDescription> _attachments = {_color_attachment};
        VkRenderPassCreateInfo render_pass_info = _getRenderPassInfo(&_attachments, &_subpass_description, &_dependency);
        
        VK_TRY(vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass));

        return;
    }


VkRenderPassBeginInfo NovaCore::getRenderPassInfo(size_t i)
    {
        //report(LOGGER::VLINE, "\t\t .. Getting Render Pass Begin Info ..");
        
        return {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = render_pass,
                .framebuffer = swapchain.framebuffers[i],
                .renderArea = {
                    .offset = {0, 0},
                    .extent = swapchain.details.extent
                },
                .clearValueCount = static_cast<uint32_t>(CLEAR_VALUES.size()),
                .pClearValues = CLEAR_VALUES.data()
            };
    }
