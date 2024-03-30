#include "gateway.h"

Gateway::Gateway()
    { clear(); }

Gateway::~Gateway()
    { clear(); }

void Gateway::clear()
    {
        _pipeline_info = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        _pipeline_layout = {};
        _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        _viewport_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        _color_blending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        _vertex_input_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        _rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        _color_blend_attachment = {};
        _multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        _pipeline_layout = {};
        _depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        _render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        _shader_stages.clear();
    }

void Gateway::createViewportState()
    {
        report(LOGGER::DLINE, "\t .. Creating Viewport State ..");

        VkExtent2D extent = _context->swapchain.extent;

        VkViewport _viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = (float)extent.width,
                .height = (float)extent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };

        VkRect2D _scissor = {
                .offset = {0, 0},
                .extent = extent
            };

        _viewport_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .viewportCount = 1,
                .pViewports = &_viewport,
                .scissorCount = 1,
                .pScissors = &_scissor
            };

    }

void Gateway::createColorBlendState()
    {
        report(LOGGER::DLINE, "\t .. Creating Color Blend State ..");

        _color_blending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &_color_blend_attachment,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
            };
    }

void Gateway::createPipelineInfo()
    {
        report(LOGGER::DLINE, "\t .. Creating Pipeline Info ..");

        _pipeline_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = 0,
                .pStages = _shader_stages.data(),
                .pVertexInputState = &_vertex_input_state,
                .pInputAssemblyState = &_input_assembly,
                .pViewportState = &_viewport_state,
                .pRasterizationState = &_rasterizer,
                .pMultisampleState = &_multisampling,
                .pColorBlendState = &_color_blending,
                .layout = _pipeline_layout
            };
        
        _pipeline_info.pDepthStencilState = &_depth_stencil;
    }

VkPipeline Gateway::pipeline()
    {
        report(LOGGER::DLINE, "\t .. Creating Pipeline ..");
        VkPipeline new_gateway;

        VK_TRY(vkCreateGraphicsPipelines(_context->logical_device, VK_NULL_HANDLE, 1, &_pipeline_info, nullptr, &new_gateway));

        return new_gateway;
    }

Gateway Gateway::build(EngineContext *context)
    {
        report(LOGGER::DLINE, "\t .. Building Pipeline ..");
        _context = context;
        createViewportState();
        createColorBlendState();
        
        _vertex_input_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        
        createPipelineInfo();

        VkDynamicState _dynamic_states[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

        VkPipelineDynamicStateCreateInfo _dynamic_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 2,
                .pDynamicStates = _dynamic_states
            };

        _pipeline_info.pDynamicState = &_dynamic_state;

        return *this;
    }


void Gateway::addShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo _shader_stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = shaderModule,
            .pName = "main"
        };

        _shader_stages.push_back(_shader_stage);
    }