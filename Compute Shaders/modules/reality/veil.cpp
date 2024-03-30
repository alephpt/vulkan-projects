
#include "./veil.h"
#include "../../components/genesis.h"

static inline VkAttachmentDescription colorAttachment(EngineContext* _context)
    {
        report(LOGGER::VLINE, "\t .. Creating Color Attachment ..");
        return {
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
        report(LOGGER::VLINE, "\t .. Creating Color Attachment Reference ..");
        return {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

static inline VkSubpassDescription subpassDescription()
    {
        report(LOGGER::VLINE, "\t .. Creating Subpass Description");

        VkAttachmentReference _color_attachment_ref = colorAttachmentRef();

        return {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &_color_attachment_ref
        };
    }

static inline VkRenderPassCreateInfo renderPassInfo(EngineContext* context)
    {
        report(LOGGER::VLINE, "\t .. Creating Render Pass Info ..");

        VkAttachmentDescription _color_attachment = colorAttachment(context);
        VkSubpassDescription _subpass_description = subpassDescription();

        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &_color_attachment,
            .subpassCount = 1,
            .pSubpasses = &_subpass_description
        };
    }

void createRenderPass(EngineContext* context)
    {
        report(LOGGER::DLINE, "\t .. Creating Render Pass ..");
        VkRenderPassCreateInfo render_pass_info = renderPassInfo(context);
        VK_TRY(vkCreateRenderPass(context->logical_device, &render_pass_info, nullptr, &context->render_pass));
    }

void constructGateway(EngineContext* context, Gateway* gateway) 
    {
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
    }
