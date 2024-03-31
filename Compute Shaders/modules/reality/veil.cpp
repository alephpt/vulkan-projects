
#include "./veil.h"
#include "../../components/genesis.h"

static inline VkAttachmentDescription colorAttachment(EngineContext* _context)
    {
        report(LOGGER::DLINE, "\t\t .. Creating Color Attachment ..");

        return {
            .flags = 0,
            .format = _context->swapchain.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }

static inline VkAttachmentReference colorAttachmentRef()
    {
        report(LOGGER::DLINE, "\t\t .. Creating Color Attachment Reference ..");

        return {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

static inline VkSubpassDescription subpassDescription(VkAttachmentReference* color_attachment_ref)
    {
        report(LOGGER::DLINE, "\t\t .. Creating Subpass Description");

        return {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = color_attachment_ref
        };
    }

static inline VkRenderPassCreateInfo renderPassInfo(VkAttachmentDescription* color_attachment, VkSubpassDescription* subpass_description)
    {
        report(LOGGER::DLINE, "\t\t .. Creating Render Pass Info ..");


        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = color_attachment,
            .subpassCount = 1,
            .pSubpasses = subpass_description
        };
    }

void createRenderPass(EngineContext* context)
    {
        report(LOGGER::ILINE, "\t .. Creating Render Pass ..");

        VkAttachmentDescription _color_attachment = colorAttachment(context);
        VkAttachmentReference _color_attachment_ref = colorAttachmentRef();
        VkSubpassDescription _subpass_description = subpassDescription(&_color_attachment_ref);
        VkRenderPassCreateInfo render_pass_info = renderPassInfo(&_color_attachment, &_subpass_description);
        VK_TRY(vkCreateRenderPass(context->logical_device, &render_pass_info, nullptr, &context->render_pass));

        return;
    }

void constructGateway(EngineContext* context, Gateway* gateway) 
    {
        gateway = new Gateway();

        gateway->define(context)
                .shaders()
                .vertexInput()
                .inputAssembly()
                .viewportState()
                .rasterizer()
                .multisampling()
                .colorBlending()
                .dynamicState()
                .layout()
                .pipeline()
                .create();

        return;
    }

void destroyGateway(Gateway* gateway) 
    {
        delete gateway;
        
        return;
    }
