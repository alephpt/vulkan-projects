#include "graphics_pipeline.h"
#include "../genesis.h"


    ////////////////////////
    // GATEWAY DEFINITION //
    ////////////////////////

GraphicsPipeline::GraphicsPipeline()
    { 
        report(LOGGER::VLINE, "\t .. Initializing Pipeline ..");
        clear(); 

        report(LOGGER::VLINE, "\t .. Populating Vertices ..");
        genesis::createObjects(&vertices, &indices); // TODO: these need to exist on a higher level, but where?
    }

GraphicsPipeline::~GraphicsPipeline()
    { 
        report(LOGGER::INFO, "Pipeline - Deconstructing Pipeline ..");
        clear();
    }

void GraphicsPipeline::clear()
    {
        report(LOGGER::VLINE, "\t .. Clearing Pipeline ..");
        instance = VK_NULL_HANDLE;
        _vertex_input_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        _viewport_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        _rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        _multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        _color_blend_attachment = {  };
        _color_blending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        _pipeline_info = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        _dynamic_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        _pipeline_layout_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        layout = {};
        _depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO }; // not used
        _render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        _shader_stages.clear();
        _shader_modules.clear();
        vertices.clear();
        indices.clear();
    }


    /////////////
    // SHADERS //
    /////////////

void GraphicsPipeline::addShaderStage(VkShaderModule shader_module, VkShaderStageFlagBits stage)
    {
        std::string stage_name = stage == VK_SHADER_STAGE_VERTEX_BIT ? "Vertex" : "Fragment";
        report(LOGGER::VLINE, "\t\t .. Adding %s Shader Stage ..", stage_name.c_str());
        _shader_modules.push_back(shader_module);

        VkPipelineShaderStageCreateInfo _shader_stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = shader_module,
            .pName = "main"
        };

        _shader_stages.push_back(_shader_stage); // We can pass this by reference and make this a static inline function

        return;
    }

// TODO: extrapolate the shaders to be passed in as a parameter, and
//       create a function that will dynamically create/update new shader modules
GraphicsPipeline& GraphicsPipeline::shaders(VkDevice* logical_device)
    {
        report(LOGGER::VLINE, "\t .. Creating Shaders ..");

        std::vector<char> _vert_shader_code = genesis::loadFile(vert_shader);
        VkShaderModule _vert_shader_module;
        genesis::createShaderModule(logical_device, _vert_shader_code, &_vert_shader_module);
        addShaderStage(_vert_shader_module, VK_SHADER_STAGE_VERTEX_BIT);

        std::vector<char> _frag_shader_code = genesis::loadFile(frag_shader);
        VkShaderModule _frag_shader_module;
        genesis::createShaderModule(logical_device, _frag_shader_code, &_frag_shader_module);
        addShaderStage(_frag_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT);

        return *this;
    }


    //////////////////
    // VERTEX INPUT //
    //////////////////

GraphicsPipeline& GraphicsPipeline::vertexInput()
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Input State ..");

        // Determine how to do this dynamically :thinking:
        // _binding_description = Vertex::getBindingDescription();
        // _attribute_descriptions = Vertex::getAttributeDescriptions();
        _binding_description = Particle::getBindingDescription();
        _attribute_descriptions = Particle::getAttributeDescriptions();

        _vertex_input_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &_binding_description,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(_attribute_descriptions.size()),
                .pVertexAttributeDescriptions = _attribute_descriptions.data()
            };

        return *this;
    }


    ////////////////////
    // INPUT ASSEMBLY //
    ////////////////////

GraphicsPipeline& GraphicsPipeline::inputAssembly()
    {
        report(LOGGER::VLINE, "\t .. Creating Input Assembly ..");

        _input_assembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST, // TRIANGLE_LIST for meshes
                .primitiveRestartEnable = VK_FALSE
            };

        return *this;
    }


    ////////////////////
    // VIEWPORT STATE //
    ////////////////////

GraphicsPipeline& GraphicsPipeline::viewportState()
    {
        report(LOGGER::VLINE, "\t .. Creating Viewport State ..");

        _viewport_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
            };

        return *this;
    }


    ////////////////
    // RASTERIZER //
    ////////////////

GraphicsPipeline& GraphicsPipeline::rasterizer()
    {
        report(LOGGER::VLINE, "\t .. Creating Rasterizer ..");

        _rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f
            };

        return *this;
    }


    ///////////////////
    // MULTISAMPLING //
    ///////////////////

GraphicsPipeline& GraphicsPipeline::multisampling(VkSampleCountFlagBits samples)
    {
        report(LOGGER::VLINE, "\t .. Creating Multisampling ..");

        _multisampling = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT, //samples,
                .sampleShadingEnable = VK_FALSE,
                //.minSampleShading = 1.0f,
                //.pSampleMask = nullptr,
                //.alphaToCoverageEnable = VK_FALSE,
                //.alphaToOneEnable = VK_FALSE
            };

        return *this;
    }


    ///////////////////
    // DEPTH STENCIL //
    ///////////////////

GraphicsPipeline& GraphicsPipeline::depthStencil()
    {
        report(LOGGER::VLINE, "\t .. Creating Depth Stencil ..");

        _depth_stencil = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .front = {},
                .back = {},
                .minDepthBounds = 0.0f,
                .maxDepthBounds = 1.0f
            };

        return *this;
    }


    /////////////////
    // COLOR BLEND //
    /////////////////

static VkPipelineColorBlendAttachmentState colorBlendAttachment()
    {
        return {
                .blendEnable = VK_TRUE, // TODO: make this dynamic
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };
    }

GraphicsPipeline& GraphicsPipeline::colorBlending()
    {
        report(LOGGER::VLINE, "\t .. Creating Color Blend State ..");

        _color_blend_attachment = colorBlendAttachment();

        _color_blending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &_color_blend_attachment,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
            };

        return *this;
    }


    ///////////////////
    // DYNAMIC STATE //
    ///////////////////

GraphicsPipeline& GraphicsPipeline::dynamicState()
    {
        report(LOGGER::VLINE, "\t .. Creating Dynamic State ..");

        _dynamic_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = static_cast<uint32_t>(_dynamic_states.size()),
                .pDynamicStates = _dynamic_states.data()
            };

        return *this;
    }


    ////////////
    // LAYOUT //
    ////////////

GraphicsPipeline& GraphicsPipeline::createLayout(VkDevice* logical_device, VkDescriptorSetLayout* descriptor_set_layout)
    {
        report(LOGGER::VLINE, "\t .. Creating Pipeline Layout ..");

        // From when we were using a vertex buffer and texture
        // _pipeline_layout_info = {
        //         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        //         .setLayoutCount = 1,
        //         .pSetLayouts = descriptor_set_layout,
        //         .pushConstantRangeCount = 0,
        //         .pPushConstantRanges = nullptr
        //     };

        // For using the Compute Pipeline
        _pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
            };

        VK_TRY(vkCreatePipelineLayout(*logical_device, &_pipeline_layout_info, nullptr, &layout));

        return *this;
    }

GraphicsPipeline& GraphicsPipeline::pipe(VkRenderPass& render_pass)
    {
        report(LOGGER::VLINE, "\t .. Creating Pipeline Create Info ..");

        _pipeline_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = 2,
                .pStages = _shader_stages.data(),
                .pVertexInputState = &_vertex_input_state,
                .pInputAssemblyState = &_input_assembly,
                .pViewportState = &_viewport_state,
                .pRasterizationState = &_rasterizer,
                .pMultisampleState = &_multisampling,
                //.pDepthStencilState = &_depth_stencil,
                .pColorBlendState = &_color_blending,
                .pDynamicState = &_dynamic_state,
                .layout = layout,
                .renderPass = render_pass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
            };

        return *this;
    }

GraphicsPipeline& GraphicsPipeline::create(VkDevice* logical_device)
    {
        report(LOGGER::VLINE, "\t .. Constructing Pipeline ..");
        VK_TRY(vkCreateGraphicsPipelines(*logical_device, VK_NULL_HANDLE, 1, &_pipeline_info, nullptr, &instance));

        report(LOGGER::VLINE, "\t .. Cleaning Up Shader Modules ..");
        for (auto shader_module : _shader_modules) 
            { vkDestroyShaderModule(*logical_device, shader_module, nullptr); }

        return *this;
    }

